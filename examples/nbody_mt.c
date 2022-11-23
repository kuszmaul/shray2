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

typedef struct
{
    Point *accel;
    Point *positions;
    Point *velocities;
    double dt;
} update_velocities_t;

typedef struct
{
    Point *accel;
    Point *positions;
    double *masses;
    size_t n;
    size_t Jstart;
    unsigned int p;
} update_accel_t;

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

void reset_accel(worker_info_t *info)
{
    Point *accel = info->args;
    for (size_t i = info->start; i < info->end; ++i) {
        accel[i].x = 0.0;
        accel[i].y = 0.0;
        accel[i].z = 0.0;
    }
}

void update_accel(worker_info_t *info)
{
    update_accel_t *data = info->args;

    size_t node_start = data->Jstart;
    size_t node_end = data->Jstart + data->n / data->p;

    fprintf(stderr, "work [%zu, %zu) (%zu, %zu)\n", info->start, info->end, node_start, node_end);
    for (size_t i = info->start; i < info->end; i++) {
        for (size_t j = node_start; j < node_end; j++) {
            data->accel[i].x +=
                accelerate(data->positions[i], data->positions[j], data->masses[j]).x;
            data->accel[i].y +=
                accelerate(data->positions[i], data->positions[j], data->masses[j]).y;
            data->accel[i].z +=
                accelerate(data->positions[i], data->positions[j], data->masses[j]).z;
        }
    }
}

void accelerateAll(Point *accel, Point *positions, double *masses, size_t n)
{
    unsigned int p = ShraySize();
    unsigned int s = ShrayRank();

    ShrayRunWorker(reset_accel, n, accel);

    /* Accelerate with respect to P((s + t) % p) and prefetch the next block. */
    for (unsigned int t = 0; t < p; t++) {
        size_t Jstart = (s + t) % p * n / p;

        update_accel_t tmp;
        tmp.accel = accel;
        tmp.positions = positions;
        tmp.masses = masses;
        tmp.n = n;
        tmp.p = p;
        tmp.Jstart = Jstart;
        ShrayRunWorker(update_accel, n, &tmp);

        if (t != 0) {
            ShrayDiscard(&positions[Jstart], n / p * sizeof(Point));
            ShrayDiscard(&masses[Jstart], n / p * sizeof(double));
        }

        if (t != p - 1) {
            ShrayPrefetch(&positions[(s + t + 1) % p * n / p], n / p * sizeof(Point));
            ShrayPrefetch(&masses[(s + t + 1) % p * n / p], n / p * sizeof(double));
        }
    }
}

void update_velocities(worker_info_t *info)
{
    update_velocities_t *tmp = info->args;
    Point *accel = tmp->accel;
    Point *positions = tmp->positions;
    Point *velocities = tmp->velocities;
    double dt = tmp->dt;

    for (size_t i = info->start; i < info->end; i++) {
        velocities[i].x += accel[i].x * dt;
        velocities[i].y += accel[i].y * dt;
        velocities[i].z += accel[i].z * dt;
        positions[i].x += velocities[i].x * dt;
        positions[i].y += velocities[i].y * dt;
        positions[i].z += velocities[i].z * dt;
    }
}

/* Advances the n-bodies in place. accel is a buffer, does not need to be initialized. */
void advance(Point *positions, Point *velocities, double *masses,
        Point *accel, double dt, size_t n)
{
    update_velocities_t tmp;

    accelerateAll(accel, positions, masses, n);
    ShraySync(accel);

    tmp.accel = accel;
    tmp.positions = positions;
    tmp.velocities = velocities;
    tmp.dt = dt;
    ShrayRunWorker(update_velocities, n, &tmp);

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
