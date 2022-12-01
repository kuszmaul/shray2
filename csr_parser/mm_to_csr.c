/* Assumes input is in the following format:
 * M N nz where the matrix is M x N and has nz non-zeroes. Then lines with
 * row number column number value, in row-major format. */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* Returns an array of files with names name1 ... namep and opens them for writing. */
FILE **createFiles(char *name, int p)
{
    FILE **fps = malloc(p * sizeof(FILE *));

    char numberString[12];

    for (int s = 1; s <= p; s++) {
        char *numberedName = malloc(strlen(name) + 12);
        strcpy(numberedName, name);
        sprintf(numberString, "%d", s);
        numberedName = strcat(numberedName, numberString);
        fps[s - 1] = fopen(numberedName, "w");
        if (fps[s - 1] == NULL) printf("Failure\n");
        free(numberedName);
    }

    return fps;
}

void closeFiles(FILE **files, int p)
{
    for (int s = 0; s < p; s++) {
        fclose(files[s]);
    }
}

int main(int argc, char **argv)
{
    if (argc != 3) {
        printf("Usage: name of file in matrix market format, number of processors\n"
               "Returns a space-seperated files with the number of non-zeroes, m, n "
               "(for an m x n matrix), row pointers (of length m + 1 where the last entry is "
               "the number of non-zeroes, column indices, values.\n");
        return EXIT_FAILURE;
    }

    int p = atoi(argv[2]);

    FILE *f;
    int M, N;
    size_t nz;
    double *val;

    if ((f = fopen(argv[1], "r")) == NULL) {
        exit(EXIT_FAILURE);
    }

    /* Header */
    if (fscanf(f, "%d %d %zu", &M, &N, &nz) != 3) return EXIT_FAILURE;
    int blockSize = (M + p - 1) / p;

    char *valuesName = malloc(strlen(argv[1]) + 7);
    char *rowName = malloc(strlen(argv[1]) + 4);
    char *columnName = malloc(strlen(argv[1]) + 7);
    char *infoName = malloc(strlen(argv[1]) + 5);

    strcpy(valuesName, argv[1]);
    valuesName = strcat(valuesName, "_values");
    strcpy(rowName, argv[1]);
    rowName = strcat(rowName, "_row");
    strcpy(columnName, argv[1]);
    columnName = strcat(columnName, "_column");
    strcpy(infoName, argv[1]);
    infoName = strcat(infoName, "_info");

    FILE **values = createFiles(valuesName, atoi(argv[2]));
    FILE **column = createFiles(columnName, atoi(argv[2]));
    FILE **row = createFiles(rowName, atoi(argv[2]));
    FILE **info = createFiles(infoName, atoi(argv[2]));

    /* Ranges from 0 to p - 1, the processor for which we build the file. */
    int processor = 0;
    /* Local row */
    int rowCounter = 1;
    /* The number of non-zeroes for the processor'th block. */
    size_t nonZeroCounter = 0;

    fprintf(row[0], "1 ");

    for (size_t i = 0; i < nz; i++) {
        nonZeroCounter++;
        int I;
        int J;
        double val;
        if (fscanf(f, "%d %d %lf\n", &I, &J, &val) != 3) EXIT_FAILURE;

        /* Matrix market indexing starts at 1. This is never true for the last processor. */
        if (I > (processor + 1) * blockSize) {
            fprintf(info[processor], "%d %d %zu", blockSize, N, nonZeroCounter - 1);

            fprintf(row[processor], "%zu", nonZeroCounter - 1);
            processor++;
            fprintf(row[processor], "1 ");
            nonZeroCounter = 1;
            rowCounter = 1;
        }

        fprintf(values[processor], "%.15lf ", val);
        fprintf(column[processor], "%d ", J);
        if (I - processor * blockSize > rowCounter) {
            fprintf(row[processor], "%zu ", nonZeroCounter);
            rowCounter++;
        }
    }

    fprintf(row[p - 1], "%zu", nonZeroCounter);
    fprintf(info[p - 1], "%d %d %zu", M - (p - 1) * blockSize, N, nonZeroCounter);

    closeFiles(values, p);
    closeFiles(column, p);
    closeFiles(row, p);
    closeFiles(info, p);
    if (f !=stdin) fclose(f);

    free(valuesName);
    free(rowName);
    free(columnName);

    return EXIT_SUCCESS;
}
