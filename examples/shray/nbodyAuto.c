#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <shray2/shray.h>

#define min(a, b) ((a) < (b) ? a : b)

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
    size_t localStart = ShrayStart(n);
    size_t localEnd = ShrayEnd(n);

    for (size_t i = localStart; i < localEnd; i++) {
        accel[i].x = 0.0;
        accel[i].y = 0.0;
        accel[i].z = 0.0;
        for (size_t j = 0; j < n; j++) {
            Point point = accelerate(positions[i], positions[j], masses[j]);
            accel[i].x += point.x;
            accel[i].y += point.y;
            accel[i].z += point.z;
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
