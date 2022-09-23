#include "../include/shray.h"
#include <math.h>
#include <assert.h>

//#define MIN(a, b) ((a) < (b)) ? (a) : (b)

typedef struct {
    double x;
    double y;
    double z;
} Point;

/* 16 flops */
Point accelerate(Point pos1, Point pos2, double mass)
{
    /* ||pos2 - pos1||^3 */
    double n = pow(pow(pos2.x - pos1.x, 2) + pow(pos2.y - pos1.y, 2) + 
                pow(pos2.z - pos1.z, 2), 1.5);

    Point a;

    a.x = (pos2.x - pos1.x) * mass / n;
    a.y = (pos2.y - pos1.y) * mass / n;
    a.z = (pos2.z - pos1.z) * mass / n;
    return a;
}

/* Same as accelerate all, but tiled for improved cache performance. */
void accelerateAllTiled(Point *accel, Point *positions, double *masses, size_t n)
{
    size_t tileSize = 1000;

    size_t start = ShrayStart(n);
    size_t end = ShrayEnd(n);

    for (size_t I = start; I < end; I += tileSize) {
        for (size_t J = 0; J < n; J += tileSize) {
            for (size_t i = I; i < MIN(I + tileSize, end); i++) {
                accel[i].x = 0.0;
                accel[i].y = 0.0;
                accel[i].z = 0.0;
                for (size_t j = J; j < MIN(J + tileSize, n); j++) {
                    accel[i].x += 
                        accelerate(positions[i], positions[j], masses[j]).x;
                    accel[i].y += 
                        accelerate(positions[i], positions[j], masses[j]).y;
                    accel[i].z += 
                        accelerate(positions[i], positions[j], masses[j]).z;
                }
            }
        }
    }
}
 
void accelerateAll(Point *accel, Point *positions, double *masses, size_t n)
{
    for (size_t i = ShrayStart(n); i < ShrayEnd(n); i++) {
        accel[i].x = 0.0;
        accel[i].y = 0.0;
        accel[i].z = 0.0;
        for (size_t j = 0; j < n; j++) {
            accel[i].x += 
                accelerate(positions[i], positions[j], masses[j]).x;
            accel[i].y += 
                accelerate(positions[i], positions[j], masses[j]).y;
            accel[i].z += 
                accelerate(positions[i], positions[j], masses[j]).z;
        }
    }
}
 
/* Advances the n-bodies in place. accel is a buffer, does not need to be initialized. */
void advance(Point *positions, Point *velocities, double *masses, 
        Point *accel, double dt, size_t n)
{
    accelerateAllTiled(positions, accel, masses, n);

    for (size_t i = ShrayStart(n); i < ShrayEnd(n); i++) {
        velocities[i].x += accel[i].x * dt;
        velocities[i].y += accel[i].y * dt;
        velocities[i].z += accel[i].z * dt;
        positions[i].x += velocities[i].x * dt;
        positions[i].y += velocities[i].y * dt;
        positions[i].z += velocities[i].z * dt;
    }
}

int main(int argc, char **argv)
{
    size_t cacheSize = 40960;

    ShrayInit(&argc, &argv, cacheSize);

    if (argc != 3) {
        printf("Usage: n, iterations\n");
        gasnet_exit(1);
    }

    size_t n = atoll(argv[1]);
    int iterations = atoi(argv[2]);

    Point *positions = ShrayMalloc(n, n * sizeof(Point));
    Point *velocities = ShrayMalloc(n, n * sizeof(Point));
    Point *accel = ShrayMalloc(n, n * sizeof(Point));
    double *masses = ShrayMalloc(n, n * sizeof(double));

    for (int i = 0; i < iterations; i++) {
        advance(positions, velocities, masses, accel, 0.1, n);
        ShraySync(positions);
        ShraySync(velocities);
    }

    ShrayFree(positions);
    ShrayFree(velocities);
    ShrayFree(masses);
    ShrayFree(accel);

    ShrayReport();

    ShrayFinalize();

    return EXIT_SUCCESS;
}
