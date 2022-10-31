#include <math.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include <shray2/shray.h>

void init(size_t n, size_t *input)
{
    for (size_t i = ShrayStart(n); i < ShrayEnd(n); i++) {
        input[i] = i;
    }
}

void random_fill(size_t n, size_t local_prob, size_t *input, size_t *output)
{
    /* Fill our portion of the output array */
    for (size_t i = ShrayStart(n); i < ShrayEnd(n); i++) {
        // TODO: Get the randomness based on a distribution by `local_prob`.
        output[i] = input[rand() % n];
    }
}

int main(int argc, char **argv)
{
    ShrayInit(&argc, &argv);

    if (argc != 3) {
        fprintf(stderr, "Usage: n, probability local access\n");
        ShrayFinalize(1);
    }

    size_t n = atoll(argv[1]);
    size_t local_prob = atoll(argv[2]);
    if (local_prob > 100) {
        fprintf(stderr, "Probability can not be greater than 100\n");
        ShrayFinalize(1);
    }

    srand(time(NULL));

    size_t *input = ShrayMalloc(n, n * sizeof(size_t));
    size_t *output = ShrayMalloc(n, n * sizeof(size_t));

    init(n, input);
    ShraySync(input);

    random_fill(n, local_prob, input, output);
    ShraySync(output);

    ShrayReport();

    ShrayFree(input);
    ShrayFree(output);

    ShrayFinalize(0);
}
