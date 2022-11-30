#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

void init(double *arr, size_t n, int p)
{
    for (size_t i = 0; i < n / p; i++) {
        arr[i] = 1;
    }
}

/* For testing only, otherwise obviously reduce locally and send your result to the
 * other nodes. Each node sums up the array. */
double reduce(double *arr, size_t n)
{
    int p;
    int s;
    MPI_Comm_size(MPI_COMM_WORLD, &p);
    MPI_Comm_rank(MPI_COMM_WORLD, &s);

    double sum = 0.0;

    double *buffer = malloc(4096);

    if (s == 0) {
        /* Add up local part */
        for (size_t i = 0; i < n / p; i++) {
            sum += arr[i];
        }

        for (int rank = 1; rank < p; rank++) {
            for (size_t page = 0; page < n / p / 512; page++) {
                MPI_Recv(buffer, 512, MPI_DOUBLE, rank, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                for (int i = 0; i < 512; i++) {
                    sum += arr[i];
                }
            }
        }
    } else {
        for (size_t page = 0; page < n / p / 512; page++) {
            MPI_Send(arr + page * 512, 512, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
        }
    }

    free(buffer);

    return sum;
}

int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);

    if (argc != 2) {
        printf("Usage: lenght of the vector you reduce\n");
        MPI_Finalize();
    }

    int p;
    MPI_Comm_size(MPI_COMM_WORLD, &p);
    size_t n = atoll(argv[1]) / p * p;

    double *A = malloc(n / p * sizeof(double));
    init(A, n, p);

    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    MPI_Barrier(MPI_COMM_WORLD);
    double start = MPI_Wtime();
    double result = reduce(A, n);

    MPI_Barrier(MPI_COMM_WORLD);
    double end = MPI_Wtime();
    double duration = end - start;

    if (rank == 0) {
        double microsPerPage = 1000000.0 * duration / (n / 512);

        fprintf(stderr, "We reduced an array on %d processors at %lf microseconds per page:\n"
                "That is a bandwidth of %lf GB/s (result is %lf)\n",
                p, microsPerPage, 4096.0 / microsPerPage / 1000.0, result);
        printf("%lf\n", microsPerPage);
    }

    free(A);
    MPI_Finalize();
}
