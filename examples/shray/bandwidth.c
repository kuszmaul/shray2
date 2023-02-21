#include <shray2/shray.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    ShrayInit(&argc, &argv);

    if (argc != 4) {
        printf("Usage: total communication volume, packet size, number of runs."
                "Packet size will be rounded to a multiple of the page size, showing a warning "
                "if this is necessary."
                "The total communication volume is rounded to a multiple of the communication "
                "without warning.\n");
        ShrayFinalize(1);
    }

    if (ShraySize() != 2) {
        printf("Please run on two processors\n");
        ShrayFinalize(1);
    }

    int numberOfRuns = atoi(argv[3]);

    size_t packetSize;

    if (atoll(argv[2]) % 4096 != 0) {
        packetSize = atoll(argv[2]) / 4096 * 4096;
        fprintf(stderr, "WARNING: packet size rounded to %zu bytes.\n", packetSize);
    } else {
        packetSize = atoll(argv[2]);
    }

    size_t arrayElements = 2 * atoll(argv[1]) / packetSize * packetSize / sizeof(double);
    size_t packetElements = packetSize / sizeof(double);

    double *array = (double *)ShrayMalloc(2 * arrayElements, 2 * arrayElements * sizeof(double));

    size_t numberOfPackets = arrayElements / 2 / packetElements;

    double duration;

    TIME(duration,
        if (ShrayRank() == 1) {
            for (int t = 0; t < numberOfRuns; t++) {
                for (size_t packet = 0; packet < numberOfPackets; packet++) {
                    ShrayPrefetch(array + packet * packetElements, packetSize);
                }

                /* We touch the first element of each packet in order to trigger the segfault
                 * that waits until the load is complete. */
                for (size_t packet = 0; packet < numberOfPackets; packet++) {
                    array[packetElements * packet] *= 2;
                }
            }
        }
    );

    if (ShrayRank() == 1) {
        double bandwidth = arrayElements * numberOfRuns * sizeof(double) /
            2 / duration / 1000000000.0;
        fprintf(stderr, "We achieve a bandwidth of %lf GB/s\n", bandwidth);
        printf("%lf", bandwidth);
    }

    ShrayFree(array);

    ShrayFinalize(0);
}
