#include <math.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <shmem.h>
#include <sys/time.h>

size_t flopCounter = 0;

#define TIME(duration, fncalls)                                        \
    {                                                                  \
        struct timeval tv1, tv2;                                       \
        gettimeofday(&tv1, NULL);                                      \
        fncalls                                                        \
        gettimeofday(&tv2, NULL);                                      \
        duration = (double) (tv2.tv_usec - tv1.tv_usec) / 1000000 +    \
         (double) (tv2.tv_sec - tv1.tv_sec);                           \
    }

#define min(a, b) ((a) < (b) ? a : b)

typedef struct {
    double x;
    double y;
    double z;
} Point;

void init(Point *positions, size_t n)
{
    int my_pe = shmem_my_pe();
    size_t elems = n / shmem_n_pes();
    for (size_t i = 0; i < elems; i++) {
        positions[i].x = i + my_pe * elems;
        positions[i].y = i + my_pe * elems;
        positions[i].z = i + my_pe * elems;
    }
}

void init_doubles(double *v, size_t n, double k)
{
    for (size_t i = 0; i < n / shmem_n_pes(); i++) {
        v[i] = k;
    }
}

/* 16 flops */
Point accelerate(Point pos1, Point pos2, double mass)
{
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
    size_t block = 200;

    int p = shmem_n_pes();

    size_t pos_buffsize = n / p * sizeof(Point);
    size_t mass_buffsize = n / p * sizeof(double);
    Point *pos_local = malloc(pos_buffsize);
    double *mass_local = malloc(mass_buffsize);

    for (size_t i = 0; i < n / p; i++) {
        accel[i].x = 0.0;
        accel[i].y = 0.0;
        accel[i].z = 0.0;
    }

    for (int s = 0; s < p; s++) {
        shmem_getmem(pos_local, positions, pos_buffsize, s);
        shmem_getmem(mass_local, masses, mass_buffsize, s);
        for (size_t Ib = 0; Ib < n / p; Ib += block) {
            for (size_t J = 0; J < n / p; J += block) {
        for (size_t i = Ib; i < min(Ib + block, n / p); i++) {
            for (size_t j = J; j < min(J + block, n / p); j++) {
                accel[i].x += accelerate(positions[i], pos_local[j], mass_local[j]).x;
                accel[i].y += accelerate(positions[i], pos_local[j], mass_local[j]).y;
                accel[i].z += accelerate(positions[i], pos_local[j], mass_local[j]).z;
//                flopCounter += 16 * 3;
            }
        }
            }
        }
    }

    free(pos_local);
    free(mass_local);
}

void advance(Point *positions, Point *velocities, double *masses,
        Point *accel, double dt, size_t n)
{
    accelerateAll(accel, positions, masses, n);

    for (size_t i = 0; i < n / shmem_n_pes(); i++) {
        velocities[i].x += accel[i].x * dt;
        velocities[i].y += accel[i].y * dt;
        velocities[i].z += accel[i].z * dt;
        positions[i].x += velocities[i].x * dt;
        positions[i].y += velocities[i].y * dt;
        positions[i].z += velocities[i].z * dt;
    }
    shmem_barrier_all();
}

int main(int argc, char **argv)
{
    if (argc != 3) {
        printf("Usage: n, iterations\n");
        return EXIT_FAILURE;
    }

    shmem_init();

    size_t n = atoll(argv[1]);
    int iterations = atoi(argv[2]);

    int p = shmem_n_pes();

    Point *positions = shmem_malloc(n / p * sizeof(Point));
    Point *velocities = shmem_malloc(n / p * sizeof(Point));
    double *masses = shmem_malloc(n / p * sizeof(double));
    Point *accel = shmem_malloc(n / p * sizeof(Point));

    init(positions, n);
    init(velocities, n);
    init_doubles(masses, n, 1);
    shmem_barrier_all();

    double duration;
    TIME(duration,
    for (int i = 0; i < iterations; i++) {
        advance(positions, velocities, masses, accel, 0.1, n);
    });

//    printf("Actual flops: %zu, expected flops: %zu\n", flopCounter * shmem_n_pes(),
//            16 * 3 * n * n * iterations);
    if (shmem_my_pe() == 0) {
        printf("%lf\n", 3.0 * 16 * n * n * iterations / 1000000000 / duration);
    }

    shmem_free(positions);
    shmem_free(velocities);
    shmem_free(masses);
    shmem_free(accel);

    shmem_finalize();

    return EXIT_SUCCESS;
}
