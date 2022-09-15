#include "../include/shray.h"
#include <stdio.h>

int main()
{
    ShrayInit(NULL, NULL, 409600);

    size_t n = 1000;

    double *arr1d = ShrayMalloc(n * n, n * n * sizeof(double));
    double *arr2d = ShrayMalloc(n, n * n * sizeof(double));

    for (size_t i = ShrayStart(n); i < ShrayEnd(n); i++) {
        for (size_t j = 0; j < n; j++) {
            arr2d[i * n + j] = 1.0;
        }
    }

    for (size_t i = ShrayStart(n * n); i < ShrayEnd(n * n); i++) {
        arr1d[i] = 1.0;
    }

    ShraySync(arr1d);
    ShraySync(arr2d);

    double sum1 = 0.0;
    double sum2 = 0.0;

    for (size_t i = 0; i < n * n; i++) {
        sum1 += arr1d[i];
        sum2 += arr2d[i];
    }

    ShrayFree(arr1d);
    ShrayFree(arr2d);

    printf("Sum of arr 1D is %lf, should be %lf.\n", sum1, (double)n * n);
    printf("Sum of arr 2D is %lf, should be %lf.\n", sum2, (double)n * n);

    ShrayReport();

    ShrayFinalize();
}
