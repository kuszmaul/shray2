#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <shray2/shray.h>

#define min(a, b) ((a) < (b) ? a : b)

size_t flopCount = 0;

typedef struct {
    double x;
    double y;
    double z;
} Point;

void init(Point *positions, size_t n)
{
    for (size_t i = ShrayStart(n); i < ShrayEnd(n); i++) {
        positions[i].x = i;
        positions[i].y = i;
        positions[i].z = i;
    }
}

/* 16 flops */
Point accelerate(Point pos1, Point pos2, double mass)
{
    /* ||pos2 - pos1||^3 */
    double n = pow(pow(pos2.x - pos1.x, 2) + pow(pos2.y - pos1.y, 2) +
                pow(pos2.z - pos1.z, 2), 1.5);

    Point a;

    if (n == 0) {
        a.x = 0;
        a.y = 0;
        a.z = 0;
    } else {
        a.x = (pos2.x - pos1.x) * mass / n;
        a.y = (pos2.y - pos1.y) * mass / n;
        a.z = (pos2.z - pos1.z) * mass / n;
    }

    return a;
}

/* Calculates the acceleration of all locally owned bodies with respect to bodies
 * [start, end[. Blocks the loops with factor block. */
void accelerateHelp(Point *accel, Point *positions, double *masses, size_t n,
        size_t start, size_t end, size_t block)
{
    size_t localStart = ShrayStart(n);
    size_t localEnd = ShrayEnd(n);

    for (size_t Ib = localStart; Ib < localEnd; Ib += block) {
        for (size_t Jb = start; Jb < end; Jb += block) {
            for (size_t i = Ib; i < min(Ib + block, localEnd); i++) {
                for (size_t j = Jb; j < min(Jb + block, end); j++) {
                    Point acceleration = accelerate(positions[i], positions[j], masses[j]);
                    accel[i].x += acceleration.x;
                    accel[i].y += acceleration.y;
                    accel[i].z += acceleration.z;
                }
            }
        }
    }
}

void accelerateAll(Point *accel, Point *positions, double *masses, size_t n)
{
    size_t localStart = ShrayStart(n);
    size_t localEnd = ShrayEnd(n);
    unsigned int p = ShraySize();
    unsigned int s = ShrayRank();

    size_t block = 200;

    for (size_t i = localStart; i < localEnd; i++) {
        accel[i].x = 0.0;
        accel[i].y = 0.0;
        accel[i].z = 0.0;
    }

    ShrayPrefetch(&positions[(s + 1) % p * n / p], n / p * sizeof(Point));
    ShrayPrefetch(&masses[(s + 1) % p * n / p], n / p * sizeof(double));

    accelerateHelp(accel, positions, masses, n, localStart, localEnd, block);

    /* Accelerate with respect to P(t) and prefetch the next block (we pretend we have a loop, so
     * P(0) comes after P(p - 1)). */
    for (unsigned int t = (s + 1) % p; t != s; t = (t + 1) % p) {
        size_t startT = t * n / p;
        size_t endT = (t + 1) * n / p;

        if ((t + 1) % p != s) {
            size_t startNext = (t + 1) % p * n / p;
            ShrayPrefetch(&positions[startNext], n / p * sizeof(Point));
            ShrayPrefetch(&masses[startNext], n / p * sizeof(double));
        }

        accelerateHelp(accel, positions, masses, n, startT, endT, block);

        ShrayDiscard(&positions[startT], n / p * sizeof(Point));
        ShrayDiscard(&masses[startT], n / p * sizeof(double));
    }
}

/* Advances the n-bodies in place. accel is a buffer, does not need to be initialized. */
void advance(Point *positions, Point *velocities, double *masses,
        Point *accel, double dt, size_t n)
{
    accelerateAll(accel, positions, masses, n);
    ShraySync(accel);

    for (size_t i = ShrayStart(n); i < ShrayEnd(n); i++) {
        velocities[i].x += accel[i].x * dt;
        velocities[i].y += accel[i].y * dt;
        velocities[i].z += accel[i].z * dt;
        positions[i].x += velocities[i].x * dt;
        positions[i].y += velocities[i].y * dt;
        positions[i].z += velocities[i].z * dt;
    }
    ShraySync(positions);
    ShraySync(velocities);
}

int main(int argc, char **argv)
{
    ShrayInit(&argc, &argv);

    if (argc != 3) {
        printf("Usage: n, iterations\n");
        ShrayFinalize(1);
    }

    size_t n = atoll(argv[1]);
    if (n % ShraySize() != 0) {
        printf("Please make sure the number of nodes divides n.\n");
        ShrayFinalize(1);
    }
    int iterations = atoi(argv[2]);

    Point *positions = (Point *)ShrayMalloc(n, n * sizeof(Point));
    Point *velocities = (Point *)ShrayMalloc(n, n * sizeof(Point));
    double *masses = (double *)ShrayMalloc(n, n * sizeof(double));
    Point *accel = (Point *)ShrayMalloc(n, n * sizeof(Point));

    init(positions, n);
    ShraySync(positions);

    double duration;

    TIME(duration, for (int i = 0; i < iterations; i++) {
        advance(positions, velocities, masses, accel, 0.1, n);
    });

    if (ShrayOutput) {
        printf("%lf\n", 3.0 * 16.0 * n * n * iterations / 1000000000 / duration);
    }

    ShrayFree(positions);
    ShrayFree(velocities);
    ShrayFree(masses);
    ShrayFree(accel);

    ShrayReport();

    ShrayFinalize(0);

    return EXIT_SUCCESS;
}
