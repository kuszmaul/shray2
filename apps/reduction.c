#include "../include/shray.h"
#include <stdio.h>
#include <pthread.h>

/* Number of threads */
int P;

#define PTHREAD_CREATE(a, b, c, d)                  \
    {                                               \
        if (!pthread_create(a, b, c, d)) {          \
            perror("Error in pthread_create\n");    \
            ShrayFinalize();                        \
        }                                           \
    }

typedef struct {
    double *array1d;
    double *array2d;
    int tid;
    size_t size;
} InitArg;

void *parallelReduce(void *arg)
{
    InitArg *input = (InitArg *)arg;

    double *sums = malloc(2 * sizeof(double));
    sums[0] = 0.0;
    sums[1] = 0.0;

    size_t totalElements = input->size * input->size;
    size_t blockSize = roundup(totalElements, P);
    size_t start = input->tid * blockSize;
    size_t end = ((input->tid + 1) * blockSize > totalElements) ? 
        totalElements : (input->tid + 1) * blockSize;

    for (size_t i = start; i < end; i++) {
        sums[0] += input->array1d[i];
        sums[1] += input->array2d[i];
    }

    return (void *)sums;
}

void *threadHello(void *arg)
{
    int *input = (int *)arg;

    printf("Hello from thread %d.\n", *input);

    return NULL;
}
void *parallelInit(void *arg)
{
    InitArg *input = (InitArg *)arg;

    /* 1d init */
    size_t n = input->size;
    size_t blockSize = roundup((ShrayEnd(n * n) - 
                ShrayStart(n * n)), P);
    size_t start = ShrayStart(n * n) + input->tid * blockSize;
    size_t end = (ShrayStart(n * n) + (input->tid + 1) * blockSize > ShrayEnd(n * n)) ? 
            ShrayEnd(n * n) : ShrayStart(n * n) + (input->tid + 1) * blockSize;

    for (size_t i = start; i < end; i++) {
        input->array1d[i] = 1.0;
    }

    /* 2d init */
    blockSize = roundup((ShrayEnd(n) - 
                ShrayStart(n)), P);
    start = ShrayStart(n) + input->tid * blockSize;
    end = (ShrayStart(n) + (input->tid + 1) * blockSize > ShrayEnd(n)) ? 
            ShrayEnd(n) : ShrayStart(n) + (input->tid + 1) * blockSize;

    for (size_t i = start; i < end; i++) {
        for (size_t j = 0; j < n; j++) {
            input->array2d[i * n + j] = 1.0;
        }
    }

    return NULL;
}

int main(int argc, char **argv)
{
    ShrayInit(&argc, &argv);

    if (argc != 3) {
        printf("Usage: square root of array size, number of threads per node.\n");
        ShrayFinalize();
    }

    size_t n = atoll(argv[1]);
    P = atoi(argv[2]);

    /* Initialisation */
    pthread_t *threads = malloc(P * sizeof(pthread_t));
    InitArg *inits = malloc(P * sizeof(InitArg));
    void **returnValues = malloc(P * sizeof(void*));

    double *arr1d = ShrayMalloc(n * n, n * n * sizeof(double));
    double *arr2d = ShrayMalloc(n, n * n * sizeof(double));

    for (int tid = 0; tid < P; tid++) {
        inits[tid].array1d = arr1d;
        inits[tid].array2d = arr2d;
        inits[tid].tid = tid;
        inits[tid].size = n;
        PTHREAD_CREATE(&threads[tid], NULL, parallelInit, &inits[tid]);
    }

    for (size_t tid = 0; tid < P; tid++) {
        pthread_join(threads[tid], returnValues[tid]);
    }

    ShraySync(arr1d);
    ShraySync(arr2d);

    /* Computation */
    void **returnSums = malloc(P * sizeof(void *));
    double sum1 = 0.0;
    double sum2 = 0.0;

    for (int tid = 0; tid < P; tid++) {
        PTHREAD_CREATE(&threads[tid], NULL, parallelReduce, &inits[tid]);
    }

    for (size_t tid = 0; tid < P; tid++) {
        pthread_join(threads[tid], returnSums[tid]);
    }

    for (size_t tid = 0; tid < P; tid++) {
        sum1 += ((double *)returnSums[tid])[0];
        sum2 += ((double *)returnSums[tid])[1];
    }

    ShrayFree(arr1d);
    ShrayFree(arr2d);
    free(threads);
    free(inits);
    free(returnValues);
    free(returnSums);

    printf("Sum of arr 1D is %lf, should be %lf.\n", sum1, (double)n * n);
    printf("Sum of arr 2D is %lf, should be %lf.\n", sum2, (double)n * n);

    ShrayReport();

    ShrayFinalize();
}
