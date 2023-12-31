#include <shray2/shray.h>
#include <stdio.h>
#include <stdlib.h>
#include "../util/time.h"

void init(double *arr)
{
    for (size_t i = ShrayStart(arr); i < ShrayEnd(arr); i++) {
        arr[i] = 1;
    }
}

/* For testing only, otherwise obviously reduce locally and send your result to
 * the other nodes. Each node sums up the array. */
double reduceAuto(double *arr, size_t n)
{
    double sum = 0.0;

    for (size_t i = ShrayEnd(arr); i < n; i++) {
        sum += arr[i];
    }

    return sum;
}

int main(int argc, char **argv)
{
    ShrayInit(&argc, &argv);

    if (argc != 2) {
        printf("Usage: lenght of the vector you reduce\n");
        ShrayFinalize(1);
    }

    size_t n = atoll(argv[1]);

    double *A = (double *)ShrayMalloc(n, n * sizeof(double));
    init(A);
    ShraySync(A);

    double duration;
    double result;


    if (ShrayOutput) {
        TIME(duration, result = reduceAuto(A, n););
        double microsPerPage = 1000000.0 * duration / ((n - ShrayEnd(A)) / 512);

        fprintf(stderr, "We reduced an array on %d processors at %lf microseconds per page:\n"
                "That is a bandwidth of %lf GB/s\n",
                ShraySize(), microsPerPage, 4096.0 / microsPerPage / 1000.0);
        printf("%lf", microsPerPage);
        fprintf(stderr, "%lf\n", result);
    }

    ShrayFree(A);

    ShrayReport();
    ShrayFinalize(0);
}
