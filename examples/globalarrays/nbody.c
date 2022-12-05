#include <ga.h>
#include <mpi.h>
#include <macdecls.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define NDIM_POINT 2
#define NDIM_MASS 1

void print_2d(int g_a)
{
	int rank = GA_Nodeid();
	if (rank == 0)  {
		return;
	}

	int lo[NDIM_POINT], hi[NDIM_POINT], ld[NDIM_POINT];
	double *a;

	NGA_Distribution(g_a, rank, lo, hi);

	NGA_Access(g_a, lo, hi, &a, ld);

	for (int i = lo[0]; i <= hi[0]; ++i) {
		for (int j = lo[1]; j <= hi[1]; ++j) {
			int k = i * ld[0] + j;
			printf("a[%f]\n", a[k]);
		}
	}
	printf("\n");

	NGA_Release(g_a, lo, hi);
}

void init_2d(int g_a)
{
	int rank = GA_Nodeid();
	int lo[NDIM_POINT], hi[NDIM_POINT], ld[NDIM_POINT];
	double *a;

	NGA_Distribution(g_a, rank, lo, hi);

	printf("%d: lo %d, %d\n", rank, lo[0], lo[1]);
	printf("%d: hi %d, %d\n", rank, hi[0], hi[1]);

	NGA_Access(g_a, lo, hi, &a, ld);
	printf("%d: ld %d, %d\n", rank, ld[0], ld[1]);

	for (int i = lo[0]; i <= hi[0]; ++i) {
		for (int j = lo[1]; j <= hi[1]; ++j) {
			int k = i * ld[0] + j;
			if (rank == 1) {
				printf("%d: %d\n", rank, k);
			}
			a[k] = k;
		}
	}

	NGA_Release(g_a, lo, hi);
}

void init_1d(int g_a)
{
	int rank = GA_Nodeid();
	int lo[NDIM_MASS], hi[NDIM_MASS], ld[NDIM_MASS];
	double *a;

	NGA_Distribution(g_a, rank, lo, hi);
	NGA_Access(g_a, lo, hi, &a, ld);

	//printf("%d: stride %d\n", rank, ld[0]);
	for (int i = lo[0]; i <= hi[0]; ++i) {
		//printf("%d ", i);
		a[i] = i;
	}
	//printf("\n");

	NGA_Release(g_a, lo, hi);
}

void accelerate(double *res, double *pos1, double *pos2, double mass)
{
    double n = pow(pow(pos2[0] - pos1[0], 2) + pow(pos2[1] - pos1[1], 2) +
                pow(pos2[2] - pos1[2], 2), 1.5);

    if (n == 0) {
        res[0] = 0;
        res[1] = 0;
        res[2] = 0;
    } else {
        res[0] += (pos2[0] - pos1[0]) * mass / n;
        res[1] += (pos2[1] - pos1[1]) * mass / n;
        res[2] += (pos2[2] - pos1[2]) * mass / n;
    }
}

void accelerateAll(int g_accel, int g_pos, int g_masses, int n)
{
	int rank = GA_Nodeid();
	int p = GA_Nnodes();

	int lo_accel[NDIM_POINT], hi_accel[NDIM_POINT], ld_accel[NDIM_POINT];
	int lo_pos[NDIM_POINT], hi_pos[NDIM_POINT], ld_pos[NDIM_POINT];

	int elems = 3;
	int pos_elems = n / p * elems;
	int mass_elems = n / p;
	int pos_buffsize = pos_elems * sizeof(double);
	int mass_buffsize = mass_elems * sizeof(double);
	double *pos_remote = malloc(pos_buffsize);
	double *mass_remote = malloc(mass_buffsize);
	double *pos_local = malloc(pos_buffsize);
	double *accel_local = malloc(pos_buffsize);

	if (!pos_remote || !mass_remote || !pos_local || !accel_local) {
		GA_Error("Could not allocate local accelerate buffers", 1);
	}

	printf("%d: --------accelAll\n", rank);

	NGA_Distribution(g_pos, rank, lo_pos, hi_pos);
	NGA_Distribution(g_accel, rank, lo_accel, hi_accel);

	ld_pos[0] = 3;
	ld_pos[1] = 0;
	ld_accel[0] = 3;
	ld_accel[1] = 0;
	NGA_Get(g_pos, lo_pos, hi_pos, pos_local, ld_pos);
	NGA_Get(g_accel, lo_accel, hi_accel, accel_local, ld_accel);

	print_2d(g_pos);
	GA_Sync(); // TODO: remove

	//printf("%d: accel: ", rank);
	for (int i = 0; i < mass_elems; ++i) {
		int k = i * ld_accel[0];
		//printf("(%d (%d) - %f %f %f)", i, k, accel_local[k], accel_local[k+1], accel_local[k+2]);
		accel_local[k] = 0; // x
		accel_local[k + 1] = 0; // y
		accel_local[k + 2] = 0; // z
	}
	//printf("\n");

	if (rank == 0)
		goto fin;

	printf("%d: (pos local) (%d, %d) (%d, %d) (%d, %d)\n", rank,
			lo_pos[0], lo_pos[1],
			hi_pos[0], hi_pos[1],
				ld_pos[0], ld_pos[1]);
	/* Get data in chunks. */
	for (int i = 0; i < n; i += mass_elems) {
		int lo_remote_mass[NDIM_MASS], hi_remote_mass[NDIM_MASS], ld_remote_mass[NDIM_MASS];
		int lo_remote_pos[NDIM_POINT], hi_remote_pos[NDIM_POINT], ld_remote_pos[NDIM_POINT];

		lo_remote_pos[0] = i;
		lo_remote_pos[1] = 0;
		hi_remote_pos[0] = i + mass_elems - 1;
		hi_remote_pos[1] = 2;

		ld_remote_pos[0] = 3;
		ld_remote_pos[1] = 0;

		lo_remote_mass[0] = i;
		hi_remote_mass[0] = i + mass_elems - 1;
		ld_remote_mass[0] = 0;

		printf("%d: (mass remote) (%d) (%d) (%d)\n", rank,
				lo_remote_mass[0],
				hi_remote_mass[0],
				ld_remote_mass[0]);
		printf("%d: (pos remote) (%d, %d) (%d, %d) (%d, %d)\n", rank,
				lo_remote_pos[0], lo_remote_pos[1],
				hi_remote_pos[0], hi_remote_pos[1],
				ld_remote_pos[0], ld_remote_pos[1]);

		NGA_Get(g_pos, lo_remote_pos, hi_remote_pos, pos_remote, ld_remote_pos);
		NGA_Get(g_masses, lo_remote_mass, hi_remote_mass, mass_remote, ld_remote_mass);

		/* Update local bodies. */
		for (int j = 0; j < mass_elems; ++j) {
			int local_offset = j * ld_accel[0];
			printf("%d: j: %d _____ ", rank, j);
			/* Go through the bodies in the retrieved chunk */
			double *tmp = pos_local + local_offset;
			printf("%d: (%f, %f, %f) ", rank, tmp[0], tmp[1], tmp[2]);

			for (int k = 0; k < mass_elems; ++k) {
				int remote_offset = k * ld_remote_pos[0];

				tmp = pos_remote + remote_offset;
				//printf("%d: (%f, %f, %f) ", rank, tmp[0], tmp[1], tmp[2]);

				accelerate(accel_local + local_offset,
					   pos_local + local_offset,
					   pos_remote + remote_offset,
					   mass_remote[k]);
			}
			printf("\n");
		}
	}
fin:

	NGA_Put(g_pos, lo_pos, hi_pos, pos_local, ld_pos);
	NGA_Put(g_accel, lo_accel, hi_accel, accel_local, ld_accel);

	free(pos_remote);
	free(mass_remote);
	free(pos_local);
	free(accel_local);
}

void advance(int g_pos, int g_vel, int g_masses, int g_accel, double dt, int n)
{
	accelerateAll(g_accel, g_pos, g_masses, n);
	GA_Sync();

	int rank = GA_Nodeid();
	double *pos, *vel, *accel;

	int lo_vel[NDIM_POINT], lo_accel[NDIM_POINT], lo_pos[NDIM_POINT];
	int hi_vel[NDIM_POINT], hi_accel[NDIM_POINT], hi_pos[NDIM_POINT];
	int ld_vel[NDIM_POINT], ld_accel[NDIM_POINT], ld_pos[NDIM_POINT];

	NGA_Distribution(g_pos, rank, lo_pos, hi_pos);
	NGA_Distribution(g_vel, rank, lo_vel, hi_vel);
	NGA_Distribution(g_accel, rank, lo_accel, hi_accel);

	//printf("%d: --------accel\n", rank);
	//printf("%d: %d, %d\n", rank, lo_vel[0], lo_vel[1]);
	//printf("%d: %d, %d\n", rank, hi_vel[0], hi_vel[1]);

	NGA_Access(g_pos, lo_pos, hi_pos, &pos, ld_pos);
	NGA_Access(g_vel, lo_vel, hi_vel, &vel, ld_vel);
	NGA_Access(g_accel, lo_accel, hi_accel, &accel, ld_accel);

	//printf("%d: stride %d, %d\n", rank, ld_vel[0], ld_vel[1]);

	for (int i = lo_vel[0]; i <= hi_vel[0]; ++i) {
		//printf("%d: i: %d _____ ", rank, i);
		for (int j = lo_vel[1]; j <= hi_vel[1]; ++j) {
			int k = i * ld_vel[0] + j;
			//printf("%d (%d), ", k, j);
			vel[k] += accel[k] * dt;
			pos[k] += vel[k] * dt;
		}
		//printf("\n");
	}

	NGA_Release(g_pos, lo_pos, hi_pos);
	NGA_Release(g_vel, lo_vel, hi_vel);
	NGA_Release(g_accel, lo_accel, hi_accel);

	GA_Sync();
}

int main(int argc, char **argv)
{
	int heap = 3000000;
	int stack = 3000000;

	MPI_Init(&argc,&argv);
	GA_Initialize();

 	if (argc != 3) {
		GA_Error("Usage: n, iterations", 1);
 	}

	/* Initialize the global allocator */
	if (!MA_init(C_DBL, stack, heap)) {
		GA_Error("MA_init failed", 1);
	}

 	int n = atoll(argv[1]);
 	int iterations = atoi(argv[2]);
	int nprocs = GA_Nnodes();

	/*
	 * GlobalArrays only supports specific types, not custom structures.
	 * Since we can't easily allocate structs we just treat it as a flat
	 * array and access them at specific offsets. We can not cast the
	 * entries to a struct to simplify programming either as structs might
	 * have padding. Treat the array of points as a two dimensional array
	 * [n, 3]
	 */
	int dimensions[NDIM_POINT] = { n, 3 };
	int chunks[NDIM_POINT] = { n / nprocs, 3 };

	int g_pos = NGA_Create(C_DBL, NDIM_POINT, dimensions, "positions", chunks);
	if (!g_pos) {
		GA_Error("Could not allocate positions", 1);
	}

	int g_vel = NGA_Duplicate(g_pos, "velocities");
	int g_accel = NGA_Duplicate(g_pos, "acceleration");
	if (!g_accel || !g_vel) {
		GA_Error("Could not allocate velocities or acceleration", 1);
	}

	/* Masses is an array of standard doubles. */
	int mass_dimensions[NDIM_MASS] = { n };
	int mass_chunks[NDIM_MASS] = { n / nprocs };
	int g_mas = NGA_Create(C_DBL, 1, mass_dimensions, "masses", mass_chunks);
	if (!g_mas) {
		GA_Error("Could not allocate masses", 1);
	}

	init_2d(g_pos);
	init_2d(g_accel);
	init_2d(g_vel);
	init_1d(g_mas);
	GA_Sync();

	for (int i = 0; i < iterations; ++i) {
		advance(g_pos, g_vel, g_mas, g_accel, 0.1, n);
	}

	GA_Destroy(g_pos);
	GA_Destroy(g_vel);
	GA_Destroy(g_accel);
	GA_Destroy(g_mas);
	GA_Terminate();
	MPI_Finalize();

	return EXIT_SUCCESS;
}
