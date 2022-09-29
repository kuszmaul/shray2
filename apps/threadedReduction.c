#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

/* Number of threads */
int P;

#define PTHREAD_CREATE(a, b, c, d)                  \
    {                                               \
        if (pthread_create(a, b, c, d) != 0) {      \
            perror("Error in pthread_create\n");    \
            exit(EXIT_FAILURE);                     \
        }                                           \
    }

#define PTHREAD_JOIN(a, b)                          \
    {                                               \
        if (pthread_join(a, b) != 0) {            \
            perror("Error in pthread_join\n");      \
            exit(EXIT_FAILURE);                     \
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
    printf("Hello from %d, reduction\n", input->tid);

    double *sums = malloc(2 * sizeof(double));
    sums[0] = 0.0;
    sums[1] = 0.0;

    size_t totalElements = input->size * input->size;
    size_t blockSize = totalElements / P;
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
    printf("Hello from %d\n", input->tid);

    /* 1d init */
    size_t n = input->size;
    size_t blockSize = (n * n + P - 1) / P;
    size_t start = input->tid * blockSize;
    size_t end = ((input->tid + 1) * blockSize > n * n) ? 
            n * n : (input->tid + 1) * blockSize;

    printf("Start %zu, end %zu\n", start, end);

    for (size_t i = start; i < end; i++) {
        input->array1d[i] = 1.0;
    }

    /* 2d init */
    blockSize = (n + P - 1) / P;
    start = input->tid * blockSize;
    end = ((input->tid + 1) * blockSize > n) ? 
            n : (input->tid + 1) * blockSize;

    for (size_t i = start; i < end; i++) {
        for (size_t j = 0; j < n; j++) {
            input->array2d[i * n + j] = 1.0;
        }
    }

    printf("Init succesfull on P(%d)\n", input->tid);

    return NULL;
}

int main(int argc, char **argv)
{
    if (argc != 3) {
        printf("Usage: square root of array size, number of threads per node.\n");
    }

    size_t n = atoll(argv[1]);
    P = atoi(argv[2]);

    /* Initialisation */
    pthread_t *threads = malloc(P * sizeof(pthread_t));
    InitArg *inits = malloc(P * sizeof(InitArg));
    void **returnValues = malloc(P * sizeof(void*));

    double *arr1d = malloc(n * n * sizeof(double));
    double *arr2d = malloc(n * n * sizeof(double));

    for (int tid = 0; tid < P; tid++) {
        inits[tid].array1d = arr1d;
        inits[tid].array2d = arr2d;
        inits[tid].tid = tid;
        inits[tid].size = n;
        PTHREAD_CREATE(&threads[tid], NULL, parallelInit, &inits[tid]);
    }

    for (size_t tid = 0; tid < P; tid++) {
        PTHREAD_JOIN(threads[tid], returnValues[tid]);
    }

    printf("Joined succesfully\n");
    /* Computation */
    void **returnSums = malloc(P * sizeof(void *));
    double sum1 = 0.0;
    double sum2 = 0.0;

    for (int tid = 0; tid < P; tid++) {
        PTHREAD_CREATE(&threads[tid], NULL, parallelReduce, &inits[tid]);
    }

    for (size_t tid = 0; tid < P; tid++) {
        PTHREAD_JOIN(threads[tid], returnSums[tid]);
    }

    for (size_t tid = 0; tid < P; tid++) {
        sum1 += ((double *)returnSums[tid])[0];
        sum2 += ((double *)returnSums[tid])[1];
    }

    free(arr1d);
    free(arr2d);
    free(threads);
    free(inits);
    free(returnValues);
    free(returnSums);

    printf("Sum of arr 1D is %lf, should be %lf.\n", sum1, (double)n * n);
    printf("Sum of arr 2D is %lf, should be %lf.\n", sum2, (double)n * n);

    return EXIT_SUCCESS;
}
