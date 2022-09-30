#include <math.h>
#include "../include/shray.h"

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

    a.x = (pos2.x - pos1.x) * mass / n;
    a.y = (pos2.y - pos1.y) * mass / n;
    a.z = (pos2.z - pos1.z) * mass / n;
    return a;
}

void accelerateAll(Point *accel, Point *positions, double *masses, size_t n)
{
    /* For segfaults it would be best to have block = 1000, for overall performance,
     * something like block = 100. Perhaps we need to block twice. */
    size_t block = 100;

    for (size_t I = ShrayStart(n); I < ShrayEnd(n); I += block) {
        for (size_t J = 0; J < n; J += block) {
            for (size_t i = I; i < min(I + block, n); i++) {
                accel[i].x = 0.0;
                accel[i].y = 0.0;
                accel[i].z = 0.0;
                for (size_t j = J; j < min(J + block, n); j++) {
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
 
/* Advances the n-bodies in place. accel is a buffer, does not need to be initialized. */
void advance(Point *positions, Point *velocities, double *masses, 
        Point *accel, double dt, size_t n)
{
    accelerateAll(positions, accel, masses, n);
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
    /* For n = 10000, we use 800000 bytes. */
    ShrayInit(&argc, &argv, 2 * 4096000 / 4);

    if (argc != 3) {
        printf("Usage: n, iterations\n");
        ShrayFinalize();
    }

    size_t n = atoll(argv[1]);
    int iterations = atoi(argv[2]);

    Point *positions = (Point *)ShrayMalloc(n, n * sizeof(Point));
    Point *velocities = (Point *)ShrayMalloc(n, n * sizeof(Point));
    double *masses = (double *)ShrayMalloc(n, n * sizeof(double));
    Point *accel = (Point *)ShrayMalloc(n, n * sizeof(Point));

    init(positions, n);
    ShraySync(positions);

    for (int i = 0; i < iterations; i++) {
        advance(positions, velocities, masses, accel, 0.1, n);
    }

    ShrayFree(positions);
    ShrayFree(velocities);
    ShrayFree(masses);
    ShrayFree(accel);

    ShrayReport();

    ShrayFinalize();

    return EXIT_SUCCESS;
}
