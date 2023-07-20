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

  Shray version: T. Koopman
  3.0 structure translation: F. Conti

  Modified to single source file, some refactoring by T. Koopman

--------------------------------------------------------------------*/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <upc.h>
#include <upc_collective.h>
#include <upc_io.h>
#include <stdbool.h>
#include <sys/time.h>
#include <omp.h>
#include <string.h>

/* For random numbers */
#define r23 pow(0.5, 23.0)
#define r46 (r23*r23)
#define t23 pow(2.0, 23.0)
#define t46 (t23*t23)

/* global variables */
char *class;
char *matdir;
shared double norm_temp11_all[THREADS];
shared double norm_temp12_all[THREADS];
shared double norm_temp11;
shared double norm_temp12;
shared double d_all[THREADS];
shared double sum_all[THREADS];
shared double rho_all[THREADS];
shared double d;
shared double sum;
shared double rho;

/* common /urando/ */
static double amult;
static double tran;

/* function declarations */
static void conj_grad(shared size_t *colidx, shared size_t *rowstr,
                      shared double *x, shared double *z, shared double *a,
                      shared double *p, shared double *q, shared double *r,
		              double *rnorm, size_t naa);

/*--------------------------------------------------------------------
 * Utilities
----------------------------------------------------------------------*/

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

size_t min(size_t a, size_t b)
{
    return (a < b) ? a : b;
}

void read_double(char *name, shared double *array, size_t n)
{
    upc_file_t *fd = upc_all_fopen(name, UPC_RDONLY|UPC_COMMON_FP, 0, NULL);

    upc_all_fread_shared(fd, array, 1, sizeof(double),
                         n, UPC_IN_ALLSYNC | UPC_OUT_ALLSYNC);

    upc_all_fclose(fd);
}

void read_size_t(char *name, shared size_t *array, size_t n)
{
    upc_file_t *fd = upc_all_fopen(name, UPC_RDONLY|UPC_COMMON_FP, 0, NULL);

    upc_all_fread_shared(fd, array, 1, sizeof(size_t),
                         n, UPC_IN_ALLSYNC | UPC_OUT_ALLSYNC);

    upc_all_fclose(fd);
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

    if (argc != 3) {
        fprintf(stderr, "Usage: %s class matdir\n", argv[0]);
        return EXIT_FAILURE;
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

    if (MYTHREAD == 0) {
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

    shared size_t *colidx = upc_all_alloc(THREADS,
                            (NZ + THREADS - 1) / THREADS * sizeof(size_t));
    shared size_t *rowstr = upc_all_alloc(THREADS,
                            (NA + 1 + THREADS - 1) / THREADS * sizeof(size_t));
    shared double *a = upc_all_alloc(THREADS,
                            (NZ + THREADS - 1) / THREADS * sizeof(double));

    shared double *x = upc_all_alloc(THREADS,
                            (NA + THREADS - 1) / THREADS * sizeof(double));
    shared double *z = upc_all_alloc(THREADS,
                            (NA + THREADS - 1) / THREADS * sizeof(double));
    shared double *p = upc_all_alloc(THREADS,
                            (NA + THREADS - 1) / THREADS * sizeof(double));
    shared double *q = upc_all_alloc(THREADS,
                            (NA + THREADS - 1) / THREADS * sizeof(double));
    shared double *r = upc_all_alloc(THREADS,
                            (NA + THREADS - 1) / THREADS * sizeof(double));

    char *name = strcatalloc("a.cg");
    read_double(name, a, NZ);
    if (MYTHREAD == 0) {
        for (int i = 0; i < 10; i++) {
            fprintf(stderr, "a[%d] = %lf\n", i, a[i]);
        }
    }
    free(name);

    name = strcatalloc("colidx.cg");
    read_size_t(name, colidx, NZ);
    if (MYTHREAD == 0) {
        for (int i = 0; i < 10; i++) {
            fprintf(stderr, "colidx[%d] = %ld\n", i, colidx[i]);
        }
    }
    free(name);

    name = strcatalloc("rowstr.cg");
    read_size_t(name, rowstr, NA + 1);
    free(name);

    upc_barrier;

/*--------------------------------------------------------------------
c  set starting vector to (1, 1, .... 1)
c-------------------------------------------------------------------*/
    size_t block = (NA + THREADS - 1) / THREADS;
    fprintf(stderr, "Thread %d: init vectors from %zu to %zu (block = %zu)\n",
        MYTHREAD, MYTHREAD * block, min((MYTHREAD + 1) * block, NA), block);
    for (size_t j = MYTHREAD * block; min((MYTHREAD + 1) * block, NA); j++) {
       x[j] = 1.0;
       q[j] = 0.0;
       z[j] = 0.0;
       r[j] = 0.0;
       p[j] = 0.0;
    }
    fprintf(stderr, "End init vectors\n");
    upc_barrier;
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
    	conj_grad (colidx, rowstr, x, z, a, p, q, r, &rnorm, NA);

    /*--------------------------------------------------------------------
    c  zeta = shift + 1/(x.z)
    c  So, first: (x.z)
    c  Also, find norm of z
    c  So, first: (z.z)
    c-------------------------------------------------------------------*/
        norm_temp11_all[MYTHREAD] = 0.0;
        norm_temp12_all[MYTHREAD] = 0.0;

        for (size_t j = MYTHREAD * block; min((MYTHREAD + 1) * block, NA); j++) {
                norm_temp11_all[MYTHREAD] = norm_temp11 + x[j]*z[j];
                norm_temp12_all[MYTHREAD] = norm_temp12 + z[j]*z[j];
    	}

        upc_all_reduceD(&norm_temp11, norm_temp11_all, UPC_ADD, THREADS, 1,
                        NULL, UPC_IN_ALLSYNC|UPC_OUT_ALLSYNC);
        upc_all_reduceD(&norm_temp12, norm_temp12_all, UPC_ADD, THREADS, 1,
                        NULL, UPC_IN_ALLSYNC|UPC_OUT_ALLSYNC);

    	norm_temp12 = 1.0 / sqrt( norm_temp12 );

    /*--------------------------------------------------------------------
    c  Normalize z to obtain x
    c-------------------------------------------------------------------*/
        for (size_t j = MYTHREAD * block; min((MYTHREAD + 1) * block, NA); j++) {
                x[j] = norm_temp12*z[j];
    	}

    } /* end of do one iteration untimed */

/*--------------------------------------------------------------------
c  set starting vector to (1, 1, .... 1)
c-------------------------------------------------------------------*/
    for (size_t j = MYTHREAD * block; min((MYTHREAD + 1) * block, NA); j++) {
         x[j] = 1.0;
    }
    upc_barrier;
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
    	conj_grad(colidx, rowstr, x, z, a, p, q, r, &rnorm, NA);

    /*--------------------------------------------------------------------
    c  zeta = shift + 1/(x.z)
    c  So, first: (x.z)
    c  Also, find norm of z
    c  So, first: (z.z)
    c-------------------------------------------------------------------*/
        norm_temp11_all[MYTHREAD] = 0.0;
        norm_temp12_all[MYTHREAD] = 0.0;

        for (size_t j = MYTHREAD * block; min((MYTHREAD + 1) * block, NA); j++) {
                norm_temp11 = norm_temp11 + x[j]*z[j];
                norm_temp12 = norm_temp12 + z[j]*z[j];
    	}

        upc_all_reduceD(&norm_temp11, norm_temp11_all, UPC_ADD, THREADS, 1,
                        NULL, UPC_IN_ALLSYNC|UPC_OUT_ALLSYNC);
        upc_all_reduceD(&norm_temp12, norm_temp12_all, UPC_ADD, THREADS, 1,
                        NULL, UPC_IN_ALLSYNC|UPC_OUT_ALLSYNC);

    	norm_temp12 = 1.0 / sqrt( norm_temp12 );

    	zeta = SHIFT + 1.0 / norm_temp11;

        if (MYTHREAD == 0) {
        	if( it == 1 ) {
        	  fprintf(stderr, "   iteration           ||r||                 zeta\n");
        	}
        	fprintf(stderr, "    %5d       %20.14e%20.13e\n", it, rnorm, zeta);
        }

    /*--------------------------------------------------------------------
    c  Normalize z to obtain x
    c-------------------------------------------------------------------*/
        for (size_t j = MYTHREAD * block; min((MYTHREAD + 1) * block, NA); j++) {
                x[j] = norm_temp12*z[j];
    	}
        upc_barrier;
    } /* end of main iter inv pow meth */

    double timer_stop = gettime();

/*--------------------------------------------------------------------
c  End of timed section
c-------------------------------------------------------------------*/

    t = timer_stop - timer_start;

    if (MYTHREAD == 0) {
        fprintf(stderr, " Benchmark completed\n");
    }

    const double epsilon = 1.0e-10;
    if (MYTHREAD == 0) {
        if (strcmp(class, "U")) {
        	if (fabs(zeta - zeta_verify_value) <= epsilon) {
        	    fprintf(stderr, " VERIFICATION SUCCESSFUL\n");
        	    fprintf(stderr, " Zeta is    %20.12e\n", zeta);
        	    fprintf(stderr, " Error is   %20.12e\n", zeta - zeta_verify_value);
    	    } else {
    	        fprintf(stderr, " VERIFICATION FAILED\n");
    	        fprintf(stderr, " Zeta                %20.12e\n", zeta);
    	        fprintf(stderr, " The correct zeta is %20.12e\n", zeta_verify_value);
		return 1;
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

    upc_barrier;

    if (MYTHREAD == 0) {
        upc_free(colidx);
        upc_free(rowstr);
        upc_free(a);
        upc_free(x);
        upc_free(z);
        upc_free(p);
        upc_free(q);
        upc_free(r);
    }

    return EXIT_SUCCESS;
}

/*--------------------------------------------------------------------
c-------------------------------------------------------------------*/
static void conj_grad (
    shared size_t *colidx,
    shared size_t *rowstr,
    shared double *x,
    shared double *z,
    shared double *a,
    shared double *p,
    shared double *q,
    shared double *r,
    double *rnorm,
    size_t naa)
/*--------------------------------------------------------------------
c-------------------------------------------------------------------*/

/*---------------------------------------------------------------------
c  Floaging point arrays here are named as in NPB1 spec discussion of
c  CG algorithm
c---------------------------------------------------------------------*/
{
    fprintf(stderr, "Enter conj_grad\n");
    static int callcount = 0;
    double rho0, alpha, beta;
    size_t j, k;
    int cgit, cgitmax = 25;

    rho = 0.0;

/*--------------------------------------------------------------------
c  Initialize the CG algorithm:
c-------------------------------------------------------------------*/
{
    size_t block = (naa + THREADS - 1) / THREADS;
    for (size_t j = MYTHREAD * block; min((MYTHREAD + 1) * block, naa); j++) {
    	q[j] = 0.0;
    	z[j] = 0.0;
    	r[j] = x[j];
    	p[j] = r[j];
    }

/*--------------------------------------------------------------------
c  rho = r.r
c  Now, obtain the norm of r: First, sum squares of r elements locally...
c-------------------------------------------------------------------*/
    for (size_t j = MYTHREAD * block; j < min((MYTHREAD + 1) * block, naa); j++) {
	    rho += r[j]*r[j];
        assert(r[j] == 1);
    }
    rho_all[MYTHREAD] = rho;
    fprintf(stderr, "rho[%d] = %lf\n", MYTHREAD, rho_all[MYTHREAD]);
    upc_all_reduceD(&rho, rho_all, UPC_ADD, THREADS, 1,
                    NULL, UPC_IN_ALLSYNC|UPC_OUT_ALLSYNC);
    fprintf(stderr, "rho = %lf\n", rho);

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

        size_t block = (naa + THREADS - 1) / THREADS;
        for (size_t j = MYTHREAD * block; j < min((MYTHREAD + 1) * block, naa); j++) {
            sum = 0.0;
    	    for (k = rowstr[j]; k < rowstr[j+1]; k++) {
    		    sum += a[k]*p[colidx[k]];
    	    }
            q[j] = sum;
    	}

        upc_barrier;

    /*--------------------------------------------------------------------
    c  Obtain p.q
    c-------------------------------------------------------------------*/
        d_all[MYTHREAD] = d;
        for (size_t j = MYTHREAD * block; j < min((MYTHREAD + 1) * block, naa); j++) {
                d_all[MYTHREAD] += p[j]*q[j];
    	}
        upc_all_reduceD(&d, d_all, UPC_ADD, THREADS, 1,
                    NULL, UPC_IN_ALLSYNC|UPC_OUT_ALLSYNC);
    /*--------------------------------------------------------------------
    c  Obtain alpha = rho / (p.q)
    c-------------------------------------------------------------------*/
    	alpha = rho0 / d;

    /*---------------------------------------------------------------------
    c  Obtain z = z + alpha*p
    c  and    r = r - alpha*q
    c---------------------------------------------------------------------*/
        rho_all[MYTHREAD] = rho;
        for (size_t j = MYTHREAD * block; j < min((MYTHREAD + 1) * block, naa); j++) {
                z[j] += alpha*p[j];
                r[j] -= alpha*q[j];

    /*---------------------------------------------------------------------
    c  rho = r.r
    c  Now, obtain the norm of r: First, sum squares of r elements locally...
    c---------------------------------------------------------------------*/
                rho_all[MYTHREAD] += r[j]*r[j];
    	}
        upc_barrier;
        upc_all_reduceD(&rho, rho_all, UPC_ADD, THREADS, 1,
                    NULL, UPC_IN_ALLSYNC|UPC_OUT_ALLSYNC);

    /*--------------------------------------------------------------------
    c  Obtain beta:
    c-------------------------------------------------------------------*/
    	beta = rho / rho0;

    /*--------------------------------------------------------------------
    c  p = r + beta*p
    c-------------------------------------------------------------------*/
        for (size_t j = MYTHREAD * block; j < min((MYTHREAD + 1) * block, naa); j++) {
                p[j] = r[j] + beta*p[j];
    	}
        upc_barrier;
        callcount++;
    } /* end of do cgit=1,cgitmax */

/*---------------------------------------------------------------------
c  Compute residual norm explicitly:  ||r|| = ||x - A.z||
c  First, form A.z
c  The partition submatrix-vector multiply
c---------------------------------------------------------------------*/
    sum = 0.0;

    size_t block = (naa + THREADS - 1) / THREADS;
    for (size_t j = MYTHREAD * block; j < min((MYTHREAD + 1) * block, naa); j++) {
    	d = 0.0;
	    for (k = rowstr[j]; k < rowstr[j+1]; k++) {
            d += a[k] * z[colidx[k]];
	    }
    	r[j] = d;
    }
    upc_barrier;

/*--------------------------------------------------------------------
c  At this point, r contains A.z
c-------------------------------------------------------------------*/
    d_all[MYTHREAD] = d;
    for (size_t j = MYTHREAD * block; j < min((MYTHREAD + 1) * block, naa); j++) {
    	d_all[MYTHREAD] = x[j] - r[j];
	    sum += d*d;
    }
    upc_all_reduceD(&d, d_all, UPC_ADD, THREADS, 1,
                    NULL, UPC_IN_ALLSYNC|UPC_OUT_ALLSYNC);
    (*rnorm) = sqrt(sum);
}
