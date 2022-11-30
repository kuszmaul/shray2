#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);

    if (argc != 4) {
        printf("Usage: total communication volume, packet size, number of runs."
                "Packet size will be rounded to a multiple of the page size, showing a warning "
                "if this is necessary."
                "The total communication volume is rounded to a multiple of the communication "
                "without warning.\n");
        MPI_Finalize();
    }

    int rank;
    int size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (size != 2) {
        printf("Please run on two processors\n");
        MPI_Finalize();
    }

    int numberOfRuns = atoi(argv[3]);

    size_t packetSize;

    if (atoll(argv[2]) % 4096 != 0) {
        packetSize = atoll(argv[2]) / 4096 * 4096;
        fprintf(stderr, "WARNING: packet size rounded to %zu bytes.\n", packetSize);
    } else {
        packetSize = atoll(argv[2]);
    }

    size_t arrayElements = atoll(argv[1]) / packetSize * packetSize / sizeof(double);
    size_t packetElements = packetSize / sizeof(double);

    double *array = malloc(arrayElements * sizeof(double));

    size_t numberOfPackets = arrayElements / packetElements;
    MPI_Request *requests = malloc(numberOfPackets * sizeof(MPI_Request));
    MPI_Status *statuses = malloc(numberOfPackets * sizeof(MPI_Status));

    MPI_Barrier(MPI_COMM_WORLD);
    double start = MPI_Wtime();

    for (int t = 0; t < numberOfRuns; t++) {
        if (rank == 0) {
            for (size_t packet = 0; packet < numberOfPackets; packet++) {
                MPI_Isend(array + packet * packetElements, packetElements, MPI_DOUBLE,
                        1, packet, MPI_COMM_WORLD, requests + packet);
            }
        }

        if (rank == 1) {
            for (size_t packet = 0; packet < numberOfPackets; packet++) {
                MPI_Irecv(array + packet * packetElements, packetElements, MPI_DOUBLE,
                        0, packet, MPI_COMM_WORLD, requests + packet);
            }
        }

        MPI_Waitall(numberOfPackets, requests, statuses);
    }

    MPI_Barrier(MPI_COMM_WORLD);
    double end = MPI_Wtime();
    double duration = end - start;

    if (rank == 1) {
        double bandwidth = arrayElements * numberOfRuns * sizeof(double) /
            2 / duration / 1000000000.0;
        fprintf(stderr, "We achieve a bandwidth of %lf GB/s\n", bandwidth);
        printf("%lf\n", bandwidth);
    }

    free(array);

    MPI_Finalize();
}
