#include <shray2/shray.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "../util/time.h"

void init(int *a)
{
    for (size_t i = ShrayStart(a); i < ShrayEnd(a); i++) {
        a[i] = i;
    }

    ShraySync(a);
}

bool test(int *a, size_t n)
{
    bool success = true;

    for (int i = 0; (size_t)i < n; i++) {
        if (a[i] != i) {
            success = false;
            printf("Node %u: a[%d] = %d\n", ShrayRank(), i, a[i]);
        }
    }

    return success;
}

int main(int argc, char **argv)
{
    ShrayInit(&argc, &argv);

    if (argc != 2) {
        printf("Usage: size of array\n");
        ShrayFinalize(1);
    }

    size_t n = atoll(argv[1]);

    int *array = ShrayMalloc(n, n * sizeof(int));
    init(array);
    if (test(array, n)) {
        printf("SUCCESS\n");
    } else {
        printf("FAILURE\n");
    }

    ShrayFree(array);

    ShrayFinalize(0);
}
