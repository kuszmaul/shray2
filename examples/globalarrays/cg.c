/*--------------------------------------------------------------------

  NAS Parallel Benchmarks 3.0 structured OpenMP C versions - CG

  This benchmark is an OpenMP C version of the NPB CG code.

  The OpenMP C 2.3 versions are derived by RWCP from the serial Fortran versions
  in "NPB 2.3-serial" developed by NAS. 3.0 translation is performed by the UVSQ.

  Permission to use, copy, distribute and modify this software for any
  purpose with or without fee is hereby granted.
  This software is provided "as is" without express or implied warranty.

  Information on OpenMP activities at RWCP is available at:

           http://pdplab.trc.rwcp.or.jp/pdperf/Omni/

  Information on NAS Parallel Benchmarks 2.3 is available at:

           http://www.nas.nasa.gov/NAS/NPB/

--------------------------------------------------------------------*/
/*--------------------------------------------------------------------

  Authors: M. Yarrow
           C. Kuszmaul

  GlobalArrays version: S. Schrijvers
  3.0 structure translation: F. Conti

--------------------------------------------------------------------*/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <sys/time.h>
#include <stdbool.h>
#include <omp.h>
#include <ga.h>
#include <mpi.h>
#include <string.h>

/* For random numbers */
#define r23 pow(0.5, 23.0)
#define r46 (r23*r23)
#define t23 pow(2.0, 23.0)
#define t46 (t23*t23)

#define MIN(a, b) (((a) < (b)) ? (a) : (b))

/* global variables */
char *class;
char *matdir;

/* common /urando/ */
static double amult;
static double tran;

/* GA distributions */
static int lo_a[1], hi_a[1], ld_a[1];
static int lo_x[1], hi_x[1], ld_x[1];
static int lo_q[1], hi_q[1], ld_q[1];
static int lo_z[1], hi_z[1], ld_z[1];
static int lo_r[1], hi_r[1], ld_r[1];
static int lo_p[1], hi_p[1], ld_p[1];
static int lo_colidx[1], hi_colidx[1], ld_colidx[1];
static int lo_rowstr[1], hi_rowstr[1], ld_rowstr[1];
static int lo_scratch[1], hi_scratch[1];
static int rank;
static int nnodes;

/* function declarations */
static void conj_grad (size_t colidx[], size_t rowstr[], double x[], int g_z,
		       double a[], int g_p, double q[], double r[],
		       double *rnorm, int g_scratch, double *scratch);

/*--------------------------------------------------------------------
 * Utilities
----------------------------------------------------------------------*/

void gaSum(double *number, double *scratch, int g_scratch)
{
    int ld[1] = { 0 };
    NGA_Put(g_scratch, lo_scratch, hi_scratch, number, ld);
    GA_Sync();
    int lo[1] = { 0 };
    int hi[1] = { nnodes } ;
    NGA_Get(g_scratch, lo, hi, scratch, ld);

    for (int i = 0; i < nnodes; i++) {
        if (i == rank) continue;
        *number += scratch[i];
    }
    GA_Sync();
}

static char *strcatalloc(const char *s)
{
    char suffix[3];
    snprintf(suffix, 3, ".%s", class);

    size_t dirlen = strlen(matdir) + 1;
    size_t slen = strlen(s);
    size_t ranklen = strlen(suffix);
    char *res = malloc(dirlen + slen + ranklen + 1);
    if (!res) {
        return NULL;
    }

    strcpy(res, matdir);
    strcat(res, "/");
    strcat(res, s);
    strcat(res, suffix);
    res[dirlen + slen + ranklen] = '\0';

    return res;
}

int read_sparse(char *name, void *array, int typewidth, int lo, int hi, int max)
{
    FILE *stream = fopen(name, "r");
    if (stream == NULL) {
        perror("Opening file failed");
        return 1;
    }

    size_t size = MIN((hi - lo) * typewidth, max);
    long offset = lo * typewidth;
    if ((fseek(stream, offset, SEEK_SET)) != 0) {
        perror("fseek failed");
        return 1;
    }

    size_t read_bytes;
    if ((read_bytes = fread(array + offset, 1, size, stream)) != size) {
        fprintf(stderr, "We could not read in all items\n");
        fprintf(stderr, "Read %zu / %zu bytes\n", read_bytes, size);
        return 1;
    }

    int err;
    if ((err = fclose(stream))) {
        perror("Closing file failed");
        return err;
    }

    return 0;
}

double gettime()
{
    struct timeval tv_start;
    gettimeofday(&tv_start, NULL);
    return (double)tv_start.tv_usec / 1000000 + (double)tv_start.tv_sec;
}

double randlc (double *x, double a) {

/*c---------------------------------------------------------------------
c---------------------------------------------------------------------*/

/*c---------------------------------------------------------------------
c
c   This routine returns a uniform pseudorandom double precision number in the
c   range (0, 1) by using the linear congruential generator
c
c   x_{k+1} = a x_k  (mod 2^46)
c
c   where 0 < x_k < 2^46 and 0 < a < 2^46.  This scheme generates 2^44 numbers
c   before repeating.  The argument A is the same as 'a' in the above formula,
c   and X is the same as x_0.  A and X must be odd double precision integers
c   in the range (1, 2^46).  The returned value RANDLC is normalized to be
c   between 0 and 1, i.e. RANDLC = 2^(-46) * x_1.  X is updated to contain
c   the new seed x_1, so that subsequent calls to RANDLC using the same
c   arguments will generate a continuous sequence.
c
c   This routine should produce the same results on any computer with at least
c   48 mantissa bits in double precision floating point data.  On 64 bit
c   systems, double precision should be disabled.
c
c   David H. Bailey     October 26, 1990
c
c---------------------------------------------------------------------*/

    double t1,t2,t3,t4,a1,a2,x1,x2,z;

/*c---------------------------------------------------------------------
c   Break A into two parts such that A = 2^23 * A1 + A2.
c---------------------------------------------------------------------*/
    t1 = r23 * a;
    a1 = (int)t1;
    a2 = a - t23 * a1;

/*c---------------------------------------------------------------------
c   Break X into two parts such that X = 2^23 * X1 + X2, compute
c   Z = A1 * X2 + A2 * X1  (mod 2^23), and then
c   X = 2^23 * Z + A2 * X2  (mod 2^46).
c---------------------------------------------------------------------*/
    t1 = r23 * (*x);
    x1 = (int)t1;
    x2 = (*x) - t23 * x1;
    t1 = a1 * x2 + a2 * x1;
    t2 = (int)(r23 * t1);
    z = t1 - t23 * t2;
    t3 = t23 * z + a2 * x2;
    t4 = (int)(r46 * t3);
    (*x) = t3 - t46 * t4;

    return (r46 * (*x));
}


/*--------------------------------------------------------------------
      program cg
--------------------------------------------------------------------*/

int main(int argc, char **argv) {

    int prov;
    MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &prov);
    if (prov != MPI_THREAD_MULTIPLE) {
    	GA_Error("MPI implementation does not support MPI_THREAD_MULTIPLE\n", 1);
    }
    GA_Initialize();

    if (argc != 3) {
        GA_Error("Usage: [program] class matdir\n", 1);
    }

    class = argv[1];
    matdir = argv[2];

    int NA, NONZER, NITER;
    double SHIFT;
    double zeta_verify_value;

    if (!strcmp(class, "S")) {
        NA = 1400;
        NONZER = 7;
        NITER = 15;
        SHIFT = 10.0;
	    zeta_verify_value = 8.5971775078648;
    } else if (!strcmp(class, "W")) {
        NA = 7000;
        NONZER = 7;
        NITER = 15;
        SHIFT = 12.0;
	    zeta_verify_value = 10.362595087124;
    } else if (!strcmp(class, "A")) {
        NA = 14000;
        NONZER = 11;
        NITER = 15;
        SHIFT = 20.0;
	    zeta_verify_value = 17.130235054029;
    } else if (!strcmp(class, "B")) {
        NA = 75000;
        NONZER = 13;
        NITER = 75;
        SHIFT = 60.0;
	    zeta_verify_value = 22.712745482631;
    } else if (!strcmp(class, "C")) {
        NA = 150000;
        NONZER = 15;
        NITER = 75;
        SHIFT = 110.0;
	    zeta_verify_value = 28.973605592845;
    } else {
        fprintf(stderr, "Error, class should be in {S, W, A, B, C}\n");
        return EXIT_FAILURE;
    }

    /* For the largest problem sizes, NA can be held by an integer, but not NZ
     * for class >= E. */
    size_t NZ = NA * (NONZER + 1) * (NONZER + 1) + NA * (NONZER + 2);

    double rnorm;
    double t, mflops;
    double norm_temp11, norm_temp12;

    if (GA_Nodeid() == 0) {
        fprintf(stderr, "\n\n NAS Parallel Benchmarks 3.0 structured Shray version"
    	   " - CG Benchmark\n");
        fprintf(stderr, " Size: %10d\n", NA);
        fprintf(stderr, " Iterations: %5d\n", NITER);
    }

/*--------------------------------------------------------------------
c  Initialize random number generator
c-------------------------------------------------------------------*/
    tran    = 314159265.0;
    amult   = 1220703125.0;
    double zeta    = randlc( &tran, amult );

/*--------------------------------------------------------------------
c
c-------------------------------------------------------------------*/

    rank = GA_Nodeid();
    nnodes = GA_Nnodes();

    int nz_chunks[1] = { (NZ + nnodes - 1) / nnodes };
    int na_chunks[1] = { (NA + nnodes - 1) / nnodes };
    int rowstr_chunks[1] = { (NA + 1 + nnodes - 1) / nnodes };

    int nz_dimensions[1] = { NZ };
    int na_dimensions[1] = { NA };
    int rowstr_dimensions[1] = { NA + 1 };

    // TODO: check if there is a workaround for GA not having size_t
    int g_colidx = NGA_Create(C_DBL, 1, nz_dimensions, "colidx", nz_chunks);
    if (!g_colidx) {
	GA_Error("Could not allocate array", 1);
    }
    int g_rowstr = NGA_Create(C_DBL, 1, rowstr_dimensions, "rowstr", rowstr_chunks);
    if (!g_rowstr) {
	GA_Error("Could not allocate array", 1);
    }
    int g_a = NGA_Duplicate(g_colidx, "a");
    if (!g_colidx) {
	GA_Error("Could not allocate array", 1);
    }

    int g_x = NGA_Create(C_DBL, 1, na_dimensions, "x", na_chunks);
    if (!g_x) {
	GA_Error("Could not allocate array", 1);
    }
    int g_z = NGA_Duplicate(g_x, "z");
    int g_p = NGA_Duplicate(g_x, "p");
    int g_q = NGA_Duplicate(g_x, "q");
    int g_r = NGA_Duplicate(g_x, "r");
    if (!g_z || !g_p || !g_q || !g_r) {
	GA_Error("Could not allocate arrays", 1);
    }

    int scratch_dimensions[1] = { nnodes };
    int g_scratch = NGA_Create(C_DBL, 1, scratch_dimensions, "scratch", NULL);
    if (!g_scratch) {
	GA_Error("Could not allocate array", 1);
    }

    double *a, *x, *q, *z, *r, *p;
    size_t *rowstr, *colidx;

    NGA_Distribution(g_a, rank, lo_a, hi_a);
    NGA_Distribution(g_x, rank, lo_x, hi_x);
    NGA_Distribution(g_q, rank, lo_q, hi_q);
    NGA_Distribution(g_z, rank, lo_z, hi_z);
    NGA_Distribution(g_r, rank, lo_r, hi_r);
    NGA_Distribution(g_p, rank, lo_p, hi_p);
    NGA_Distribution(g_rowstr, rank, lo_rowstr, hi_rowstr);
    NGA_Distribution(g_colidx, rank, lo_colidx, hi_colidx);
    NGA_Distribution(g_scratch, rank, lo_scratch, hi_scratch);
    NGA_Access(g_a, lo_a, hi_a, &a, ld_a);
    NGA_Access(g_x, lo_a, hi_x, &x, ld_x);
    NGA_Access(g_q, lo_q, hi_q, &q, ld_q);
    NGA_Access(g_r, lo_r, hi_r, &r, ld_r);
    NGA_Access(g_rowstr, lo_rowstr, hi_rowstr, &rowstr, ld_rowstr);
    NGA_Access(g_colidx, lo_colidx, hi_colidx, &colidx, ld_colidx);

    char *name = strcatalloc("a.cg");
    if (read_sparse(name, a, sizeof(double), lo_a[0], hi_a[0], nz_dimensions[0])) {
        fprintf(stderr, "Reading %s went wrong\n", name);
	    GA_Error("Failed to read file", 1);
    }
    free(name);

    name = strcatalloc("colidx.cg");
    if (read_sparse(name, colidx, sizeof(size_t), lo_colidx[0], hi_colidx[0], nz_dimensions[0])) {
        fprintf(stderr, "Reading %s went wrong\n", name);
	    GA_Error("Failed to read file", 1);
    }
    free(name);

    name = strcatalloc("rowstr.cg");
    if (read_sparse(name, rowstr, sizeof(size_t), lo_rowstr[0], hi_rowstr[0], rowstr_dimensions[0])) {
        fprintf(stderr, "Reading %s went wrong\n", name);
	    GA_Error("Failed to read file", 1);
    }
    free(name);

    GA_Sync();

/*--------------------------------------------------------------------
c  set starting vector to (1, 1, .... 1)
c-------------------------------------------------------------------*/
    for (size_t i = lo_x[0]; i < hi_x[0]; i++) {
    	x[i] = 1.0;
    }

    NGA_Access(g_p, lo_p, hi_p, &p, ld_p);
    for (size_t j = lo_q[0]; j < hi_q[0]; j++) {
       q[j] = 0.0;
       z[j] = 0.0;
       r[j] = 0.0;
       p[j] = 0.0;
    }
    NGA_Release_update(g_p, lo_p, hi_p);
    GA_Sync();
    zeta  = 0.0;

/*-------------------------------------------------------------------
c---->
c  Do one iteration untimed to init all code and data page tables
c---->                    (then reinit, start timing, to niter its)
c-------------------------------------------------------------------*/

    double *scratch = malloc(nnodes * sizeof(double));

    for (int it = 1; it <= 1; it++) {

    /*--------------------------------------------------------------------
    c  The call to the conjugate gradient routine:
    c-------------------------------------------------------------------*/
    	conj_grad (colidx, rowstr, x, g_z, a, g_p, q, r, &rnorm, g_scratch, scratch);

    /*--------------------------------------------------------------------
    c  zeta = shift + 1/(x.z)
    c  So, first: (x.z)
    c  Also, find norm of z
    c  So, first: (z.z)
    c-------------------------------------------------------------------*/
    	norm_temp11 = 0.0;
    	norm_temp12 = 0.0;
        NGA_Access(g_z, lo_z, hi_z, &z, ld_z);
    	for (int j = lo_x[0]; j < hi_x[0]; j++) {
                norm_temp11 = norm_temp11 + x[j]*z[j];
                norm_temp12 = norm_temp12 + z[j]*z[j];
    	}
	gaSum(&norm_temp11, scratch, g_scratch);
	gaSum(&norm_temp12, scratch, g_scratch);
    	norm_temp12 = 1.0 / sqrt( norm_temp12 );

    /*--------------------------------------------------------------------
    c  Normalize z to obtain x
    c-------------------------------------------------------------------*/
    	for (int j = lo_x[0]; j < hi_x[0]; j++) {
                x[j] = norm_temp12*z[j];
    	}

    	NGA_Release(g_z, lo_z, hi_z);
    } /* end of do one iteration untimed */

/*--------------------------------------------------------------------
c  set starting vector to (1, 1, .... 1)
c-------------------------------------------------------------------*/
    for (int i = lo_x[0]; i < hi_x[0]; i++) {
         x[i] = 1.0;
    }
    zeta  = 0.0;

    GA_Sync();
    double timer_start = gettime();

/*--------------------------------------------------------------------
c---->
c  Main Iteration for inverse power method
c---->
c-------------------------------------------------------------------*/

    for (int it = 1; it <= NITER; it++) {

    /*--------------------------------------------------------------------
    c  The call to the conjugate gradient routine:
    c-------------------------------------------------------------------*/
    	conj_grad(colidx, rowstr, x, g_z, a, g_p, q, r, &rnorm, g_scratch, scratch);

    /*--------------------------------------------------------------------
    c  zeta = shift + 1/(x.z)
    c  So, first: (x.z)
    c  Also, find norm of z
    c  So, first: (z.z)
    c-------------------------------------------------------------------*/
    	norm_temp11 = 0.0;
    	norm_temp12 = 0.0;

        NGA_Access(g_z, lo_z, hi_z, &z, ld_z);
    	for (int j = lo_x[0]; j < hi_x[0]; j++) {
                norm_temp11 = norm_temp11 + x[j]*z[j];
                norm_temp12 = norm_temp12 + z[j]*z[j];
    	}
    	gaSum(&norm_temp11, scratch, g_scratch);
        gaSum(&norm_temp12, scratch, g_scratch);

    	norm_temp12 = 1.0 / sqrt( norm_temp12 );

    	zeta = SHIFT + 1.0 / norm_temp11;

        if (rank == 0) {
        	if( it == 1 ) {
        	  fprintf(stderr, "   iteration           ||r||                 zeta\n");
        	}
        	fprintf(stderr, "    %5d       %20.14e%20.13e\n", it, rnorm, zeta);
        }

    /*--------------------------------------------------------------------
    c  Normalize z to obtain x
    c-------------------------------------------------------------------*/
    	for (int j = lo_x[0]; j < hi_x[0]; j++) {
                x[j] = norm_temp12*z[j];
    	}
    	NGA_Release(g_z, lo_z, hi_z);
        GA_Sync();
    } /* end of main iter inv pow meth */

    double timer_stop = gettime();

/*--------------------------------------------------------------------
c  End of timed section
c-------------------------------------------------------------------*/

    t = timer_stop - timer_start;

    if (rank == 0) {
        fprintf(stderr, " Benchmark completed\n");
    }

    const double epsilon = 1.0e-10;
    if (rank == 0) {
        if (strcmp(class, "U")) {
        	if (fabs(zeta - zeta_verify_value) <= epsilon) {
        	    fprintf(stderr, " VERIFICATION SUCCESSFUL\n");
        	    fprintf(stderr, " Zeta is    %20.12e\n", zeta);
        	    fprintf(stderr, " Error is   %20.12e\n", zeta - zeta_verify_value);
    	    } else {
    	        fprintf(stderr, " VERIFICATION FAILED\n");
    	        fprintf(stderr, " Zeta                %20.12e\n", zeta);
    	        fprintf(stderr, " The correct zeta is %20.12e\n", zeta_verify_value);
		GA_Error("Failed verification", 1);
    	    }
        } else {
    	    fprintf(stderr, " Problem size unknown\n");
    	    fprintf(stderr, " NO VERIFICATION PERFORMED\n");
        }

        if ( t != 0.0 ) {
    	mflops = (2.0*NITER*NA)
    	    * (3.0+(NONZER*(NONZER+1)) + 25.0*(5.0+(NONZER*(NONZER+1))) + 3.0 )
    	    / t / 1000000.0;
        } else {
    	mflops = 0.0;
        }

        printf("%lf", mflops / 1000.0);
        fprintf(stderr, "%lf Gflops/s\n", mflops / 1000.0);
    }

    NGA_Release(g_colidx, lo_colidx, hi_colidx);
    NGA_Release(g_rowstr, lo_rowstr, hi_rowstr);
    NGA_Release(g_a, lo_a, hi_a);
    NGA_Release_update(g_q, lo_q, hi_q);
    NGA_Release_update(g_p, lo_p, hi_p);
    NGA_Release_update(g_z, lo_z, hi_z);
    NGA_Release_update(g_r, lo_r, hi_r);

    NGA_Destroy(g_colidx);
    NGA_Destroy(g_rowstr);
    NGA_Destroy(g_a);

    NGA_Destroy(g_x);
    NGA_Destroy(g_z);
    NGA_Destroy(g_p);
    NGA_Destroy(g_q);
    NGA_Destroy(g_r);

    free(scratch);

    GA_Terminate();
    MPI_Finalize();
}

/*--------------------------------------------------------------------
c-------------------------------------------------------------------*/
static void conj_grad (
    size_t colidx[],
    size_t rowstr[],
    double x[],
    int g_z,
    double a[],
    int g_p,
    double q[],
    double r[],
    double *rnorm,
    int g_scratch,
    double *scratch)
/*--------------------------------------------------------------------
c-------------------------------------------------------------------*/

/*---------------------------------------------------------------------
c  Floaging point arrays here are named as in NPB1 spec discussion of
c  CG algorithm
c---------------------------------------------------------------------*/
{
    static int callcount = 0;
    double d, sum, rho, rho0, alpha, beta;
    size_t j, k;
    double *p, *z;
    int cgit, cgitmax = 25;
    rho = 0.0;

/*--------------------------------------------------------------------
c  Initialize the CG algorithm:
c-------------------------------------------------------------------*/
{
    NGA_Access(g_p, lo_p, hi_p, &p, ld_p);
    NGA_Access(g_z, lo_z, hi_z, &z, ld_z);
    for (j = lo_q[0]; j < hi_q[0]; j++) {
    	q[j] = 0.0;
    	z[j] = 0.0;
    	r[j] = x[j];
    	p[j] = r[j];
    }
    NGA_Release_update(g_p, lo_p, hi_p);
    GA_Sync();

/*--------------------------------------------------------------------
c  rho = r.r
c  Now, obtain the norm of r: First, sum squares of r elements locally...
c-------------------------------------------------------------------*/
    for (j = lo_r[0]; j < hi_r[0]; j++) {
	    rho += r[j]*r[j];
    }
    gaSum(&rho, scratch, g_scratch);
}/* end omp parallel */
/*--------------------------------------------------------------------
c---->
c  The conj grad iteration loop
c---->
c-------------------------------------------------------------------*/
    for (cgit = 1; cgit <= cgitmax; cgit++) {
        rho0 = rho;
        d = 0.0;
        rho = 0.0;

    /*--------------------------------------------------------------------
    c  q = A.p
    c  The partition submatrix-vector multiply: use workspace w
    c---------------------------------------------------------------------
    C
    C  NOTE: this version of the multiply is actually (slightly: maybe %5)
    C        faster on the sp2 on 16 nodes than is the unrolled-by-2 version
    C        below.   On the Cray t3d, the reverse is true, i.e., the
    C        unrolled-by-two version is some 10% faster.
    C        The unrolled-by-8 version below is significantly faster
    C        on the Cray t3d - overall speed of code is 1.5 times faster.
    */

        for (j = lo_q[0]; j < hi_q[0]; j++) {
            sum = 0.0;
    	    for (k = rowstr[j]; k < rowstr[j+1]; k++) {
                    double val;
                    size_t col = colidx[k];
                    int lo_val[1] = { col };
                    int hi_val[1] = { col };
                    int ld_val[1] = { 0 };
                    NGA_Get(g_p, lo_val, hi_val, &val, ld_val);
    		    sum += a[k] * val;
    	    }
            q[j] = sum;
    	}

    /*--------------------------------------------------------------------
    c  Obtain p.q
    c-------------------------------------------------------------------*/
        NGA_Access(g_p, lo_p, hi_p, &p, ld_p);
    	for (j = lo_p[0]; j < hi_p[0]; j++) {
                d += p[j]*q[j];
    	}
        gaSum(&d, scratch, g_scratch);
    /*--------------------------------------------------------------------
    c  Obtain alpha = rho / (p.q)
    c-------------------------------------------------------------------*/
    	alpha = rho0 / d;

    /*---------------------------------------------------------------------
    c  Obtain z = z + alpha*p
    c  and    r = r - alpha*q
    c---------------------------------------------------------------------*/
    	for (j = lo_z[0]; j < hi_z[0]; j++) {
                z[j] += alpha*p[j];
                r[j] -= alpha*q[j];

    /*---------------------------------------------------------------------
    c  rho = r.r
    c  Now, obtain the norm of r: First, sum squares of r elements locally...
    c---------------------------------------------------------------------*/
                rho += r[j]*r[j];
    	}
        gaSum(&rho, scratch, g_scratch);


    /*--------------------------------------------------------------------
    c  Obtain beta:
    c-------------------------------------------------------------------*/
    	beta = rho / rho0;

    /*--------------------------------------------------------------------
    c  p = r + beta*p
    c-------------------------------------------------------------------*/
    	for (j = lo_p[0]; j < hi_p[0]; j++) {
                p[j] = r[j] + beta*p[j];
    	}
        callcount++;
        NGA_Release_update(g_p, lo_p, hi_p);
        GA_Sync();
    } /* end of do cgit=1,cgitmax */


    NGA_Release_update(g_z, lo_z, hi_z);
/*---------------------------------------------------------------------
c  Compute residual norm explicitly:  ||r|| = ||x - A.z||
c  First, form A.z
c  The partition submatrix-vector multiply
c---------------------------------------------------------------------*/
    sum = 0.0;

    // TODO: Z needs to be updated afterwards
    for (j = lo_r[0]; j < hi_r[0]; j++) {
    	d = 0.0;
	    for (k = rowstr[j]; k < rowstr[j+1]; k++) {
                    double val;
                    size_t col = colidx[k];
                    int lo_val[1] = { col };
                    int hi_val[1] = { col };
                    int ld_val[1] = { 0 };
                    NGA_Get(g_z, lo_val, hi_val, &val, ld_val);
                    d += a[k] * val;
	    }
    	r[j] = d;
    }

/*--------------------------------------------------------------------
c  At this point, r contains A.z
c-------------------------------------------------------------------*/
    for (j = lo_x[0]; j < hi_x[0]; j++) {
    	d = x[j] - r[j];
	    sum += d*d;
    }
    gaSum(&sum, scratch, g_scratch);
    (*rnorm) = sqrt(sum);
}
