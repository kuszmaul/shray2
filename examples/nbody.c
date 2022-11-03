#include <math.h>
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

void accelerateAll(Point *accel, Point *positions, double *masses, size_t n)
{
    size_t start = ShrayStart(n);
    size_t end = ShrayEnd(n);
    unsigned int p = ShraySize();
    unsigned int s = ShrayRank();

    /* For segfaults it would be best to have block = 1000. For physical cache performance
     * something like block = 100. So that is why we block twice. */
    size_t block = 200;

    for (size_t i = start; i < end; i++) {
        accel[i].x = 0.0;
        accel[i].y = 0.0;
        accel[i].z = 0.0;
    }

    /* Accelerate with respect to P((s + t) % p) and prefetch the next block. */
    for (unsigned int t = 0; t < p; t++) {
        size_t Jstart = (s + t) % p * n / p;
        if (t != 0) {
            ShrayGetComplete(&positions[Jstart]);
            ShrayGetComplete(&masses[Jstart]);
        }

        for (size_t Ib = start; Ib < end; Ib += block) {
            for (size_t J = Jstart; J < Jstart + n / p; J += block) {
                for (size_t i = Ib; i < min(Ib + block, end); i++) {
                    for (size_t j = J; j < min(J + block, Jstart + n / p); j++) {
                        accel[i].x +=
                            accelerate(positions[i], positions[j], masses[j]).x;
                        accel[i].y +=
                            accelerate(positions[i], positions[j], masses[j]).y;
                        accel[i].z +=
                            accelerate(positions[i], positions[j], masses[j]).z;
                        flopCount += 16 * 3;
                    }
                }
            }
        }

        if (t != 0) {
            ShrayGetFree(&positions[Jstart]);
            ShrayGetFree(&masses[Jstart]);
        }
        
        if (t != p - 1) {
            ShrayGet(&positions[(s + t + 1) % p * n / p], n / p * sizeof(Point));
            ShrayGet(&masses[(s + t + 1) % p * n / p], n / p * sizeof(double));
        }
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

    printf("Expected flops %zu, actual flops %zu\n", 
            16 * 3 * n * n * iterations, flopCount * ShraySize());

    if (ShrayOutput) {
        printf("Time: %lfs, %lf Gflops/s\n", duration, 
                16.0 * n * n * iterations / 1000000000 / duration);
    }

    ShrayFree(positions);
    ShrayFree(velocities);
    ShrayFree(masses);
    ShrayFree(accel);

    ShrayReport();

    ShrayFinalize(0);

    return EXIT_SUCCESS;
}
