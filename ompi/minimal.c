#include <mpi.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/mman.h>

MPI_Win win;

void bufferedGet(void *origin, size_t count,
    size_t owner_rank, size_t offset)
{
    MPI_Win_lock(MPI_LOCK_SHARED, owner_rank, 0, win);
    MPI_Get(origin, count, MPI_BYTE, owner_rank,
                       offset, count,
                       MPI_BYTE, win);
    MPI_Win_unlock(owner_rank, win);
}

int main(int argc, char **argv)
{
   MPI_Init(&argc, &argv);

   size_t size = 409600;
   void *ptr = mmap(NULL, size, PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
   MPI_Win_create(ptr, size, 1, MPI_INFO_NULL, MPI_COMM_WORLD, &win);

   void *result = malloc(10);

   bufferedGet(result, 10, 0, 1);

   free(result);
   MPI_Win_free(&win);
   munmap(ptr, size);

   MPI_Finalize();
}
