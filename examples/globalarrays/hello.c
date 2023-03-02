#include <ga.h>
#include <mpi.h>
#include <macdecls.h>

int main(int argc, char **argv)
{
	MPI_Init(&argc,&argv); /* start MPI */
	GA_Initialize(); /* start global arrays */

	int p = GA_Nodeid();
	int n = GA_Nnodes();
	printf("Hello world %d (%d)\n", p, n);

	GA_Terminate(); /* tidy up global arrays call */
	MPI_Finalize(); /* tidy up MPI */

	return 0;
}
