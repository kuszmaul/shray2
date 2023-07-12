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

  Shray version by T. Koopman

--------------------------------------------------------------------*/

/*
c---------------------------------------------------------------------
c  Note: please observe that in the routine conj_grad three
c  implementations of the sparse matrix-vector multiply have
c  been supplied.  The default matrix-vector multiply is not
c  loop unrolled.  The alternate implementations are unrolled
c  to a depth of 2 and unrolled to a depth of 8.  Please
c  experiment with these to find the fastest for your particular
c  architecture.  If reporting timing results, any of these three may
c  be used without penalty.
c---------------------------------------------------------------------
*/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <sys/time.h>
#include <stdbool.h>
#include <shray2/shray.h>
#include <omp.h>
#include <string.h>
#include <gasnet.h>
#include <gasnet_coll.h>
#include <errno.h>


/* For random numbers */
#define r23 pow(0.5, 23.0)
#define r46 (r23*r23)
#define t23 pow(2.0, 23.0)
#define t46 (t23*t23)

/* global variables */

/* common /partit_size/ */
static char *class;
static int naa;
static int nzz;
static int firstrow;
static int lastrow;
static int firstcol;
static int lastcol;

/* common /urando/ */
static double amult;
static double tran;

/* function declarations */
static void conj_grad (int colidx[], int rowstr[], double x[], double z[],
		       double a[], double p[], double q[], double r[],
		       //double w[],
		       double *rnorm);

/*--------------------------------------------------------------------
 * Utilities
----------------------------------------------------------------------*/

void gasnetBarrier(void)
{
    gasnet_barrier_notify(0, GASNET_BARRIERFLAG_ANONYMOUS);
    gasnet_barrier_wait(0, GASNET_BARRIERFLAG_ANONYMOUS);
}

/* Collective operation, replaces number by its sum over
 * all nodes. */
void gasnetSum(double *number)
{
    double *numbers = malloc(ShraySize() * sizeof(double));
    gasnet_coll_gather_all(gasnete_coll_team_all, numbers, number,
            sizeof(double), GASNET_COLL_DST_IN_SEGMENT);
    for (size_t i = 0; i < ShraySize(); i++) {
        if (i == ShrayRank()) continue;
        *number += numbers[i];
    }
}

int cg_read_a(double *a)
{
    printf("Rank %d is reading a\n", ShrayRank());
    char name[50];
    sprintf(name, "a.cg.%s_%d", class, ShrayRank());
    FILE *stream = fopen(name, "r");
    if (stream == NULL) {
        perror("Opening a.cg failed");
        return 1;
    }

    char *line = NULL;
    size_t len = 0;
    ssize_t nread;

    size_t i = 0;
    while ((nread = getline(&line, &len, stream)) != -1) {
        a[i] = atof(line);
        i++;
    }
    printf("Rank %d read %lu lines\n", ShrayRank(), i);

    free(line);
    int err;
    if (!(err = fclose(stream))) {
        perror("Closing a.cg failed");
        return err;
    }

    return i - (nzz + 1);
}

int cg_read_col(int *colidx)
{
    char name[50];
    sprintf(name, "colidx.cg.%s_%d", class, ShrayRank());
    FILE *stream = fopen(name, "r");
    if (stream == NULL) {
        perror("Opening colidx.cg failed");
        return 1;
    }

    char *line = NULL;
    size_t len = 0;
    ssize_t nread;

    size_t i = 0;
    while ((nread = getline(&line, &len, stream)) != -1) {
        colidx[i] = atoi(line);
        i++;
    }

    free(line);
    int err;
    if (!(err = fclose(stream))) {
        perror("Closing colidx.cg failed");
        return err;
    }

    return i - (nzz + 1);
}

int cg_read_row(int *rowstr)
{
    char name[50];
    sprintf(name, "rowstr.cg.%s_%d", class, ShrayRank());
    FILE *stream = fopen(name, "r");
    if (stream == NULL) {
        perror("Opening rowstr.cg failed");
        return 1;
    }

    char *line = NULL;
    size_t len = 0;
    ssize_t nread;

    size_t i = 0;
    while ((nread = getline(&line, &len, stream)) != -1) {
        rowstr[i] = atoi(line);
        i++;
    }

    free(line);
    int err;
    if (!(err = fclose(stream))) {
        perror("Closing rowstr.cg failed");
        return err;
    }

    return i - (naa + 2);
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

int max(int a, int b)
{
    return (a > b) ? a : b;
}

int min(int a, int b)
{
    return (a < b) ? a : b;
}

/*--------------------------------------------------------------------
      program cg
--------------------------------------------------------------------*/

int main(int argc, char **argv) {

    if (argc != 2) {
        fprintf(stderr, "Usage: %s class\n", argv[0]);
        return EXIT_FAILURE;
    }

    ShrayInit(&argc, &argv);

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

    int NZ = NA * (NONZER + 1) * (NONZER + 1) + NA * (NONZER + 2);

    int	i, j, k, it;
    double zeta;
    double rnorm;
    double norm_temp11;
    double norm_temp12;
    double t, mflops;
    double epsilon;

    firstrow = 1;
    lastrow  = NA;
    firstcol = 1;
    lastcol  = NA;

    fprintf(stderr, "\n\n NAS Parallel Benchmarks 3.0 structured Shray C version"
	   " - CG Benchmark\n");
    fprintf(stderr, " Size: %10d\n", NA);
    fprintf(stderr, " Iterations: %5d\n", NITER);

    naa = NA;
    nzz = NZ;

/*--------------------------------------------------------------------
c  Initialize random number generator
c-------------------------------------------------------------------*/
    tran    = 314159265.0;
    amult   = 1220703125.0;
    zeta    = randlc( &tran, amult );

/*--------------------------------------------------------------------
c
c-------------------------------------------------------------------*/

    int *colidx = ShrayMalloc(NZ+1, (NZ+1) * sizeof(int));	/* colidx[1:NZ] */
    int *rowstr = ShrayMalloc(NA+1+1, (NA+1+1) * sizeof(int));	/* rowstr[1:NA+1] */

    double *a = ShrayMalloc(NZ+1, (NZ+1) * sizeof(double));		/* a[1:NZ] */
    double *x = ShrayMalloc(NA+2+1, (NA+2+1) * sizeof(double));	/* x[1:NA+2] */
    double *z = ShrayMalloc(NA+2+1, (NA+2+1) * sizeof(double));	/* z[1:NA+2] */
    double *p = ShrayMalloc(NA+2+1, (NA+2+1) * sizeof(double));	/* p[1:NA+2] */
    double *q = ShrayMalloc(NA+2+1, (NA+2+1) * sizeof(double));	/* q[1:NA+2] */
    double *r = ShrayMalloc(NA+2+1, (NA+2+1) * sizeof(double));	/* r[1:NA+2] */

    cg_read_a(a + ShrayStart(a));
    cg_read_col(colidx + ShrayStart(colidx));
    cg_read_row(rowstr + ShrayStart(rowstr));

/*---------------------------------------------------------------------
c  Note: as a result of the above call to makea:
c        values of j used in indexing rowstr go from 1 --> lastrow-firstrow+1
c        values of colidx which are col indexes go from firstcol --> lastcol
c        So:
c        Shift the col index vals from actual (firstcol --> lastcol )
c        to local, i.e., (1 --> lastcol-firstcol+1)
c---------------------------------------------------------------------*/
	for (k = max(ShrayStart(colidx), rowstr[1]);
            k < min(ShrayEnd(colidx), rowstr[lastrow - firstrow + 1]); k++) {
        colidx[k] = colidx[k] - firstcol + 1;
	}
    ShraySync(colidx);

/*--------------------------------------------------------------------
c  set starting vector to (1, 1, .... 1)
c-------------------------------------------------------------------*/
    for (i = max(ShrayStart(x), 1); i < min(ShrayEnd(x), NA+2); i++) {
	    x[i] = 1.0;
    }
    for (j = max(ShrayStart(q), 1); j < min(ShrayStart(q), lastcol-firstcol+2); j++) {
       q[j] = 0.0;
       z[j] = 0.0;
       r[j] = 0.0;
       p[j] = 0.0;
    }
    ShraySync(x);
    ShraySync(q);
    ShraySync(z);
    ShraySync(r);
    ShraySync(p);
    zeta  = 0.0;

/*-------------------------------------------------------------------
c---->
c  Do one iteration untimed to init all code and data page tables
c---->                    (then reinit, start timing, to niter its)
c-------------------------------------------------------------------*/

    for (it = 1; it <= 1; it++) {

/*--------------------------------------------------------------------
c  The call to the conjugate gradient routine:
c-------------------------------------------------------------------*/
	conj_grad (colidx, rowstr, x, z, a, p, q, r,/* w,*/ &rnorm);

/*--------------------------------------------------------------------
c  zeta = shift + 1/(x.z)
c  So, first: (x.z)
c  Also, find norm of z
c  So, first: (z.z)
c-------------------------------------------------------------------*/
	norm_temp11 = 0.0;
	norm_temp12 = 0.0;
	for (j = max(ShrayStart(x), 1);
            j < min(ShrayEnd(x), lastcol-firstcol+2); j++) {
        norm_temp11 = norm_temp11 + x[j]*z[j];
        norm_temp12 = norm_temp12 + z[j]*z[j];
	}
    gasnetBarrier();
    gasnetSum(&norm_temp11);
    gasnetSum(&norm_temp12);

	norm_temp12 = 1.0 / sqrt( norm_temp12 );

/*--------------------------------------------------------------------
c  Normalize z to obtain x
c-------------------------------------------------------------------*/
	for (j = max(ShrayStart(x), 1); j < min(ShrayEnd(x), lastcol-firstcol+2); j++) {
            x[j] = norm_temp12*z[j];
	}
    ShraySync(x);

    } /* end of do one iteration untimed */

/*--------------------------------------------------------------------
c  set starting vector to (1, 1, .... 1)
c-------------------------------------------------------------------*/
    for (i = max(ShrayStart(x), 1); i < min(ShrayEnd(x), NA+2); i++) {
         x[i] = 1.0;
    }
    ShraySync(x);
    zeta  = 0.0;


    struct timeval tv_start;
    gettimeofday(&tv_start, NULL);
    double timer_start = (double)tv_start.tv_usec / 1000000 +
                        (double)tv_start.tv_sec;

/*--------------------------------------------------------------------
c---->
c  Main Iteration for inverse power method
c---->
c-------------------------------------------------------------------*/

    for (it = 1; it <= NITER; it++) {

/*--------------------------------------------------------------------
c  The call to the conjugate gradient routine:
c-------------------------------------------------------------------*/
	conj_grad(colidx, rowstr, x, z, a, p, q, r/*, w*/, &rnorm);

/*--------------------------------------------------------------------
c  zeta = shift + 1/(x.z)
c  So, first: (x.z)
c  Also, find norm of z
c  So, first: (z.z)
c-------------------------------------------------------------------*/
	norm_temp11 = 0.0;
	norm_temp12 = 0.0;

#pragma omp parallel for default(shared) private(j) reduction(+:norm_temp11,norm_temp12)

	for (j = max(ShrayStart(x), 1); j < min(ShrayEnd(x), lastcol-firstcol+2); j++) {
            norm_temp11 = norm_temp11 + x[j]*z[j];
            norm_temp12 = norm_temp12 + z[j]*z[j];
	}
    gasnetBarrier();
    gasnetSum(&norm_temp11);
    gasnetSum(&norm_temp12);

	norm_temp12 = 1.0 / sqrt( norm_temp12 );

	zeta = SHIFT + 1.0 / norm_temp11;

	if( it == 1 ) {
	  fprintf(stderr, "   iteration           ||r||                 zeta\n");
	}
	fprintf(stderr, "    %5d       %20.14e%20.13e\n", it, rnorm, zeta);

/*--------------------------------------------------------------------
c  Normalize z to obtain x
c-------------------------------------------------------------------*/
	for (j = max(ShrayStart(x), 1); j < min(ShrayEnd(x), lastcol-firstcol+2); j++) {
            x[j] = norm_temp12*z[j];
	}
    ShraySync(x);
    } /* end of main iter inv pow meth */

    struct timeval tv_stop;
    gettimeofday(&tv_stop, NULL);
    double timer_stop = (double)tv_stop.tv_usec / 1000000 +
                        (double)tv_stop.tv_sec;

/*--------------------------------------------------------------------
c  End of timed section
c-------------------------------------------------------------------*/

    t = timer_stop - timer_start;

    fprintf(stderr, " Benchmark completed\n");

    epsilon = 1.0e-10;
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

    ShrayFinalize(0);
}

/*--------------------------------------------------------------------
c-------------------------------------------------------------------*/
static void conj_grad (
    int colidx[],	/* colidx[1:nzz] */
    int rowstr[],	/* rowstr[1:naa+1] */
    double x[],		/* x[*] */
    double z[],		/* z[*] */
    double a[],		/* a[1:nzz] */
    double p[],		/* p[*] */
    double q[],		/* q[*] */
    double r[],		/* r[*] */
    double *rnorm )
/*--------------------------------------------------------------------
c-------------------------------------------------------------------*/

/*---------------------------------------------------------------------
c  Floaging point arrays here are named as in NPB1 spec discussion of
c  CG algorithm
c---------------------------------------------------------------------*/
{
    static int callcount = 0;
    double d, sum, rho, rho0, alpha, beta;
    int j, k;
    int cgit, cgitmax = 25;

    rho = 0.0;

/*--------------------------------------------------------------------
c  Initialize the CG algorithm:
c-------------------------------------------------------------------*/
    for (j = max(ShrayStart(q), 1); j < min(ShrayEnd(q), naa+2); j++) {
	q[j] = 0.0;
	z[j] = 0.0;
	r[j] = x[j];
	p[j] = r[j];
	//w[j] = 0.0;
    }
    ShraySync(q);
    ShraySync(z);
    ShraySync(r);
    ShraySync(p);

/*--------------------------------------------------------------------
c  rho = r.r
c  Now, obtain the norm of r: First, sum squares of r elements locally...
c-------------------------------------------------------------------*/
    for (j = max(ShrayStart(r), 1);
            j < min(ShrayEnd(r), lastcol-firstcol+2); j++) {
	    rho = rho + r[j]*r[j];
    }
    gasnetBarrier();
    gasnetSum(&rho);
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

/* rolled version */
	for (j = max(ShrayStart(q), 1); j < min(ShrayEnd(q), lastrow-firstrow+2); j++) {
        sum = 0.0;
	    for (k = rowstr[j]; k < rowstr[j+1]; k++) {
		    sum = sum + a[k]*p[colidx[k]];
	    }
            //w[j] = sum;
            q[j] = sum;
	}
    ShraySync(q);

/*--------------------------------------------------------------------
c  Obtain p.q
c-------------------------------------------------------------------*/
	for (j = max(ShrayStart(p), 1);
            j < min(ShrayEnd(p), lastcol-firstcol+2); j++) {
        d = d + p[j]*q[j];
	}
    gasnetBarrier();
    gasnetSum(&d);
/*--------------------------------------------------------------------
c  Obtain alpha = rho / (p.q)
c-------------------------------------------------------------------*/
//#pragma omp single
	alpha = rho0 / d;

/*--------------------------------------------------------------------
c  Save a temporary of rho
c-------------------------------------------------------------------*/
	/*	rho0 = rho;*/

/*---------------------------------------------------------------------
c  Obtain z = z + alpha*p
c  and    r = r - alpha*q
c---------------------------------------------------------------------*/
	for (j = max(ShrayStart(z), 1);
            j < min(ShrayEnd(z), lastcol-firstcol+2); j++) {
        z[j] = z[j] + alpha*p[j];
        r[j] = r[j] - alpha*q[j];
        rho = rho + r[j]*r[j];
	}
    gasnetBarrier();
    gasnetSum(&rho);

/*--------------------------------------------------------------------
c  Obtain beta:
c-------------------------------------------------------------------*/
	beta = rho / rho0;

/*--------------------------------------------------------------------
c  p = r + beta*p
c-------------------------------------------------------------------*/
	for (j = max(ShrayStart(p), 1);
            j < min(ShrayEnd(p), lastcol-firstcol+2); j++) {
        p[j] = r[j] + beta*p[j];
	}
    ShraySync(p);
    callcount++;
    } /* end of do cgit=1,cgitmax */

/*---------------------------------------------------------------------
c  Compute residual norm explicitly:  ||r|| = ||x - A.z||
c  First, form A.z
c  The partition submatrix-vector multiply
c---------------------------------------------------------------------*/
    sum = 0.0;

    for (j = max(ShrayStart(r), 1); j < min(ShrayEnd(r), lastrow-firstrow+2); j++) {
    	d = 0.0;
    	for (k = rowstr[j]; k <= rowstr[j+1]-1; k++) {
                d = d + a[k]*z[colidx[k]];
    	}
    	r[j] = d;
    }
    ShraySync(r);

/*--------------------------------------------------------------------
c  At this point, r contains A.z
c-------------------------------------------------------------------*/
    for (j = max(ShrayStart(x), 1);
            j < min(ShrayEnd(x), lastcol-firstcol+2); j++) {
    	d = x[j] - r[j];
	    sum = sum + d*d;
    }
    gasnetBarrier();
    gasnetSum(&sum);
    (*rnorm) = sqrt(sum);
}
