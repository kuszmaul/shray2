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

  OpenMP C version: S. Satoh
  3.0 structure translation: F. Conti

  Modified to single source file, some refactoring by T. Koopman

--------------------------------------------------------------------*/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <sys/time.h>
#include <stdbool.h>
#include <omp.h>
#include <string.h>

/* For random numbers */
#define r23 pow(0.5, 23.0)
#define r46 (r23*r23)
#define t23 pow(2.0, 23.0)
#define t46 (t23*t23)

/* global variables */
char *class;

/* common /urando/ */
static double amult;
static double tran;

/* function declarations */
static void conj_grad (size_t colidx[], size_t rowstr[], double x[], double z[],
		       double a[], double p[], double q[], double r[],
		       double *rnorm, int naa);

/*--------------------------------------------------------------------
 * Utilities
----------------------------------------------------------------------*/

int read_sparse(char *name, void *array, size_t size)
{
    FILE *stream = fopen(name, "r");
    if (stream == NULL) {
        perror("Opening file failed");
        return 1;
    }

    size_t read_bytes;
    if ((read_bytes = fread(array, 1, size, stream)) != size) {
        printf("We could not read in all items");
        printf("Read %zu / %zu bytes\n", read_bytes, size);
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
    if (argc != 2) {
        fprintf(stderr, "Usage: %s class\n", argv[0]);
        return EXIT_FAILURE;
    }

    class = argv[1];

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

    int	i, j;
    double rnorm;
    double t, mflops;
    double norm_temp11, norm_temp12;

    fprintf(stderr, "\n\n NAS Parallel Benchmarks 3.0 structured OpenMP C version"
	   " - CG Benchmark\n");
    fprintf(stderr, " Size: %10d\n", NA);
    fprintf(stderr, " Iterations: %5d\n", NITER);

    int naa = NA;

/*--------------------------------------------------------------------
c  Initialize random number generator
c-------------------------------------------------------------------*/
    tran    = 314159265.0;
    amult   = 1220703125.0;
    double zeta    = randlc( &tran, amult );

/*--------------------------------------------------------------------
c
c-------------------------------------------------------------------*/

    size_t *colidx = malloc(NZ * sizeof(size_t));
    size_t *rowstr = malloc((NA + 1) * sizeof(size_t));
    int *iv = malloc(2 * NA * sizeof(int));
    int *arow = malloc(NZ * sizeof(int));
    int *acol = malloc(NZ * sizeof(int));

    double *v = malloc((NA + 1) * sizeof(double));
    double *aelt = malloc(NZ * sizeof(double));
    double *x = malloc(NA * sizeof(double));
    double *z = malloc(NA * sizeof(double));
    double *p = malloc(NA * sizeof(double));
    double *q = malloc(NA * sizeof(double));
    double *r = malloc(NA * sizeof(double));
    double *a = malloc(NZ * sizeof(double));

    char name[50];

    sprintf(name, "a.cg.%s", class);
    if (read_sparse(name, a, NZ * sizeof(double))) {
        fprintf(stderr, "Reading %s went wrong\n", name);
    }

    sprintf(name, "colidx.cg.%s", class);
    if (read_sparse(name, colidx, NZ * sizeof(size_t))) {
        fprintf(stderr, "Reading %s went wrong\n", name);
    }

    sprintf(name, "rowstr.cg.%s", class);
    if (read_sparse(name, rowstr, (NA + 1) * sizeof(size_t))) {
        fprintf(stderr, "Reading %s went wrong\n", name);
    }

#pragma omp parallel default(shared) private(i,j)
{
/*--------------------------------------------------------------------
c  set starting vector to (1, 1, .... 1)
c-------------------------------------------------------------------*/
#pragma omp for nowait
    for (i = 0; i < NA; i++) {
    	x[i] = 1.0;
    }
#pragma omp for nowait
    for (j = 0; j < NA; j++) {
       q[j] = 0.0;
       z[j] = 0.0;
       r[j] = 0.0;
       p[j] = 0.0;
    }
}// end omp parallel
    zeta  = 0.0;

/*-------------------------------------------------------------------
c---->
c  Do one iteration untimed to init all code and data page tables
c---->                    (then reinit, start timing, to niter its)
c-------------------------------------------------------------------*/

    for (int it = 1; it <= 1; it++) {

/*--------------------------------------------------------------------
c  The call to the conjugate gradient routine:
c-------------------------------------------------------------------*/
	conj_grad (colidx, rowstr, x, z, a, p, q, r, &rnorm, naa);

/*--------------------------------------------------------------------
c  zeta = shift + 1/(x.z)
c  So, first: (x.z)
c  Also, find norm of z
c  So, first: (z.z)
c-------------------------------------------------------------------*/
	norm_temp11 = 0.0;
	norm_temp12 = 0.0;
    #pragma omp parallel for default(shared) private(j) reduction(+:norm_temp11,norm_temp12)
	for (j = 0; j < NA; j++) {
            norm_temp11 = norm_temp11 + x[j]*z[j];
            norm_temp12 = norm_temp12 + z[j]*z[j];
	}
	norm_temp12 = 1.0 / sqrt( norm_temp12 );

/*--------------------------------------------------------------------
c  Normalize z to obtain x
c-------------------------------------------------------------------*/
    #pragma omp parallel for default(shared) private(j)
	for (j = 0; j < NA; j++) {
            x[j] = norm_temp12*z[j];
	}

    } /* end of do one iteration untimed */

/*--------------------------------------------------------------------
c  set starting vector to (1, 1, .... 1)
c-------------------------------------------------------------------*/
#pragma omp parallel for default(shared) private(i)
    for (i = 0; i < NA; i++) {
         x[i] = 1.0;
    }
    zeta  = 0.0;


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
	conj_grad(colidx, rowstr, x, z, a, p, q, r, &rnorm, naa);

/*--------------------------------------------------------------------
c  zeta = shift + 1/(x.z)
c  So, first: (x.z)
c  Also, find norm of z
c  So, first: (z.z)
c-------------------------------------------------------------------*/
	norm_temp11 = 0.0;
	norm_temp12 = 0.0;

#pragma omp parallel for default(shared) private(j) reduction(+:norm_temp11,norm_temp12)
	for (j = 0; j < NA; j++) {
            norm_temp11 = norm_temp11 + x[j]*z[j];
            norm_temp12 = norm_temp12 + z[j]*z[j];
	}

	norm_temp12 = 1.0 / sqrt( norm_temp12 );

	zeta = SHIFT + 1.0 / norm_temp11;

	if( it == 1 ) {
	  fprintf(stderr, "   iteration           ||r||                 zeta\n");
	}
	fprintf(stderr, "    %5d       %20.14e%20.13e\n", it, rnorm, zeta);

/*--------------------------------------------------------------------
c  Normalize z to obtain x
c-------------------------------------------------------------------*/
#pragma omp parallel for default(shared) private(j)
	for (j = 0; j < NA; j++) {
            x[j] = norm_temp12*z[j];
	}
    } /* end of main iter inv pow meth */

    double timer_stop = gettime();

/*--------------------------------------------------------------------
c  End of timed section
c-------------------------------------------------------------------*/

    t = timer_stop - timer_start;

    fprintf(stderr, " Benchmark completed\n");

    const double epsilon = 1.0e-10;
    if (strcmp(class, "U")) {
    	if (fabs(zeta - zeta_verify_value) <= epsilon) {
    	    fprintf(stderr, " VERIFICATION SUCCESSFUL\n");
    	    fprintf(stderr, " Zeta is    %20.12e\n", zeta);
    	    fprintf(stderr, " Error is   %20.12e\n", zeta - zeta_verify_value);
	    } else {
	        fprintf(stderr, " VERIFICATION FAILED\n");
	        fprintf(stderr, " Zeta                %20.12e\n", zeta);
	        fprintf(stderr, " The correct zeta is %20.12e\n", zeta_verify_value);
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

    free(colidx);
    free(rowstr);
    free(iv);
    free(arow);
    free(acol);
    free(aelt);

    free(v);
    free(a);
    free(x);
    free(z);
    free(p);
    free(q);
    free(r);
}

/*--------------------------------------------------------------------
c-------------------------------------------------------------------*/
static void conj_grad (
    size_t colidx[],
    size_t rowstr[],
    double x[],
    double z[],
    double a[],
    double p[],
    double q[],
    double r[],
    double *rnorm,
    int naa)
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
    int cgit, cgitmax = 25;

    rho = 0.0;
#pragma omp parallel default(shared) private(j,sum) shared(rho,naa)

/*--------------------------------------------------------------------
c  Initialize the CG algorithm:
c-------------------------------------------------------------------*/
{
#pragma omp for
    for (j = 0; j < naa; j++) {
    	q[j] = 0.0;
    	z[j] = 0.0;
    	r[j] = x[j];
    	p[j] = r[j];
    }

/*--------------------------------------------------------------------
c  rho = r.r
c  Now, obtain the norm of r: First, sum squares of r elements locally...
c-------------------------------------------------------------------*/
#pragma omp for reduction(+:rho)
    for (j = 0; j < naa; j++) {
	    rho += r[j]*r[j];
    }
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
#pragma omp parallel default(shared) private(j,k,sum,alpha,beta) shared(d,rho0,rho)
{

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

/* rolled version */
#pragma omp for
	for (j = 0; j < naa; j++) {
        sum = 0.0;
	    for (k = rowstr[j]; k < rowstr[j+1]; k++) {
		    sum += a[k]*p[colidx[k]];
	    }
        q[j] = sum;
	}

/*--------------------------------------------------------------------
c  Obtain p.q
c-------------------------------------------------------------------*/
#pragma omp for reduction(+:d)
	for (j = 0; j < naa; j++) {
            d += p[j]*q[j];
	}
#pragma omp barrier
/*--------------------------------------------------------------------
c  Obtain alpha = rho / (p.q)
c-------------------------------------------------------------------*/
	alpha = rho0 / d;

/*---------------------------------------------------------------------
c  Obtain z = z + alpha*p
c  and    r = r - alpha*q
c---------------------------------------------------------------------*/
#pragma omp for reduction(+:rho)
	for (j = 0; j < naa; j++) {
            z[j] += alpha*p[j];
            r[j] -= alpha*q[j];

/*---------------------------------------------------------------------
c  rho = r.r
c  Now, obtain the norm of r: First, sum squares of r elements locally...
c---------------------------------------------------------------------*/
            rho += r[j]*r[j];
	}

/*--------------------------------------------------------------------
c  Obtain beta:
c-------------------------------------------------------------------*/
	beta = rho / rho0;

/*--------------------------------------------------------------------
c  p = r + beta*p
c-------------------------------------------------------------------*/
#pragma omp for nowait
	for (j = 0; j < naa; j++) {
            p[j] = r[j] + beta*p[j];
	}
    callcount++;
    } /* end omp parallel */
    } /* end of do cgit=1,cgitmax */

/*---------------------------------------------------------------------
c  Compute residual norm explicitly:  ||r|| = ||x - A.z||
c  First, form A.z
c  The partition submatrix-vector multiply
c---------------------------------------------------------------------*/
    sum = 0.0;

#pragma omp parallel default(shared) private(j,d) shared(sum)
{
#pragma omp for //private(d, k)
    for (j = 0; j < naa; j++) {
    	d = 0.0;
	    for (k = rowstr[j]; k < rowstr[j+1]; k++) {
            d += a[k] * z[colidx[k]];
	    }
    	r[j] = d;
    }

/*--------------------------------------------------------------------
c  At this point, r contains A.z
c-------------------------------------------------------------------*/
#pragma omp for reduction(+:sum)
    for (j = 0; j < naa; j++) {
    	d = x[j] - r[j];
	    sum += d*d;
    }
}
    (*rnorm) = sqrt(sum);
}
