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
static void conj_grad (int colidx[], int rowstr[], double x[], double z[],
		       double a[], double p[], double q[], double r[],
		       double *rnorm, int naa);
static void makea(int n, double a[], int colidx[], int rowstr[],
		  int nonzer, double rcond, int arow[], int acol[],
		  double aelt[], double v[], int iv[], double shift );
static void sparse(double a[], int colidx[], int rowstr[], int n,
		   int arow[], int acol[], double aelt[],
		   double x[], int mark[], int nzloc[], int nnza);
static void sprnvc(int n, int nz, double v[], int iv[], int nzloc[],
		   int mark[]);
static int icnvrt(double x, int ipwr2);
static void vecset(double v[], int iv[], int *nzv, int i, double val);

/*--------------------------------------------------------------------
 * Utilities
----------------------------------------------------------------------*/

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
    double RCOND = 1.0e-1;
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

    int *colidx = malloc(NZ * sizeof(int));
    int *rowstr = malloc((NA + 1) * sizeof(int));
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

    makea(naa, a, colidx, rowstr, NONZER,
	  RCOND, arow, acol, aelt, v, iv, SHIFT);

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
    int colidx[],
    int rowstr[],
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
    int j, k;
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
		    sum += a[k - 1]*p[colidx[k - 1] - 1];
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
            d += a[k - 1]*z[colidx[k - 1] - 1];
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

/*---------------------------------------------------------------------
c       generate the test problem for benchmark 6
c       makea generates a sparse matrix with a
c       prescribed sparsity distribution
c
c       parameter    type        usage
c
c       input
c
c       n            i           number of cols/rows of matrix
c       rcond        r*8         condition number
c       shift        r*8         main diagonal shift
c
c       output
c
c       a            r*8         array for nonzeros
c       colidx       i           col indices
c       rowstr       i           row pointers
c
c       workspace
c
c       iv, arow, acol i
c       v, aelt        r*8
c---------------------------------------------------------------------*/
static void makea(
    int n,
    double a[],
    int colidx[],
    int rowstr[],
    int nonzer,
    double rcond,
    int arow[],
    int acol[],
    double aelt[],
    double v[],
    int iv[],
    double shift )
{
    int i, nnza, iouter, ivelt, ivelt1, irow, nzv;

/*--------------------------------------------------------------------
c      nonzer is approximately  (int(sqrt(nnza /n)));
c-------------------------------------------------------------------*/

    double size, ratio, scale;
    int jcol;

    size = 1.0;
    ratio = pow(rcond, (1.0 / (double)n));
    nnza = 0;

/*---------------------------------------------------------------------
c  Initialize colidx(n .. 2n - 1) to zero.
c  Used by sprnvc to mark nonzero positions
c---------------------------------------------------------------------*/
#pragma omp parallel for default(shared) private(i)
    for (i = 0; i < n; i++) {
    	colidx[n+i] = 0;
    }
    for (iouter = 1; iouter <= n; iouter++) {
	    nzv = nonzer;
	    sprnvc(n, nzv, v, iv, &(colidx[0]), &(colidx[n]));
	    vecset(v, iv, &nzv, iouter, 0.5);
	    for (ivelt = 0; ivelt < nzv; ivelt++) {
	        jcol = iv[ivelt];
	    	scale = size * v[ivelt];
	    	for (ivelt1 = 0; ivelt1 < nzv; ivelt1++) {
	            irow = iv[ivelt1];
	    		acol[nnza] = jcol;
	    		arow[nnza] = irow;
	    		aelt[nnza] = v[ivelt1] * scale;
	    		nnza++;
	    	}
	    }
	    size *= ratio;
    }

/*---------------------------------------------------------------------
c       ... add the identity * rcond to the generated matrix to bound
c           the smallest eigenvalue from below by rcond
c---------------------------------------------------------------------*/
    for (i = 1; i <= n; i++) {
    	iouter = n + i;
    	acol[nnza] = i;
    	arow[nnza] = i;
    	aelt[nnza] = rcond - shift;
    	nnza++;
    }

/*---------------------------------------------------------------------
c       ... make the sparse matrix from list of elements with duplicates
c           (v and iv are used as  workspace)
c---------------------------------------------------------------------*/
    sparse(a, colidx, rowstr, n, arow, acol, aelt,
	   v, &(iv[0]), &(iv[n]), nnza);
}

/*---------------------------------------------------
c       generate a sparse matrix from a list of
c       [col, row, element] tri
c---------------------------------------------------*/
static void sparse(
    double a[],
    int colidx[],
    int rowstr[],
    int n,
    int arow[],
    int acol[],
    double aelt[],
    double x[],
    int mark[],	/* mark[1:n] */
    int nzloc[],	/* nzloc[1:n] */
    int nnza)
/*---------------------------------------------------------------------
c       rows range from 1 to n
c       the rowstr pointers are defined for nrows = n values
c---------------------------------------------------------------------*/
{
    int nrows;
    int i, j, jajp1, nza, k, nzrow;
    double xi;

/*--------------------------------------------------------------------
c    how many rows of result
c-------------------------------------------------------------------*/
    nrows = n;

/*--------------------------------------------------------------------
c     ...count the number of triples in each row
c-------------------------------------------------------------------*/
#pragma omp parallel for default(shared) private(j)
    for (j = 1; j <= n; j++) {
	    rowstr[j - 1] = 0;
	    mark[j] = false;
    }
    rowstr[n] = 0;

    for (nza = 0; nza < nnza; nza++) {
	    j = arow[nza] + 1;
	    rowstr[j - 1] = rowstr[j - 1] + 1;
    }

    rowstr[0] = 1;
    for (j = 1; j <= nrows; j++) {
	    rowstr[j] = rowstr[j] + rowstr[j-1];
    }

/*---------------------------------------------------------------------
c     ... rowstr(j) now is the location of the first nonzero
c           of row j of a
c---------------------------------------------------------------------*/

/*---------------------------------------------------------------------
c     ... preload data pages
c---------------------------------------------------------------------*/
#pragma omp parallel for default(shared) private(k,j)
      for(j = 0; j < nrows; j++) {
         for (k = rowstr[j]; k < rowstr[j+1]; k++)
	       a[k - 1] = 0.0;
      }
/*--------------------------------------------------------------------
c     ... do a bucket sort of the triples on the row index
c-------------------------------------------------------------------*/
    for (nza = 0; nza < nnza; nza++) {
	    j = arow[nza] - 1;
	    k = rowstr[j];
	    a[k - 1] = aelt[nza];
	    colidx[k - 1] = acol[nza];
	    rowstr[j] = rowstr[j] + 1;
    }

/*--------------------------------------------------------------------
c       ... rowstr(j) now points to the first element of row j+1
c-------------------------------------------------------------------*/
    for (j = nrows; j >= 1; j--) {
	    rowstr[j] = rowstr[j - 1];
    }
    rowstr[0] = 1;

/*--------------------------------------------------------------------
c       ... generate the actual output rows by adding elements
c-------------------------------------------------------------------*/
    nza = 0;
#pragma omp parallel for default(shared) private(i)
    for (i = 1; i <= n; i++) {
	    x[i] = 0.0;
	    mark[i] = false;
    }

    jajp1 = rowstr[0];
    for (j = 1; j <= nrows; j++) {
	    nzrow = 0;

/*--------------------------------------------------------------------
c              ...loop over the jth row of a
c-------------------------------------------------------------------*/
	    for (k = jajp1; k < rowstr[j]; k++) {
            i = colidx[k - 1];
            x[i] += a[k - 1];
            if ( mark[i] == false && x[i] != 0.0) {
	    	    mark[i] = true;
	    	    nzrow = nzrow + 1;
	    	    nzloc[nzrow] = i;
	        }
	    }

/*--------------------------------------------------------------------
c              ... extract the nonzeros of this row
c-------------------------------------------------------------------*/
	    for (k = 1; k <= nzrow; k++) {
            i = nzloc[k];
            mark[i] = false;
            xi = x[i];
            x[i] = 0.0;
            if (xi != 0.0) {
	    	    a[nza] = xi;
	    	    colidx[nza] = i;
	    	    nza = nza + 1;
	        }
	    }
	    jajp1 = rowstr[j];
	    rowstr[j] = nza + rowstr[0];
    }
}

/*---------------------------------------------------------------------
c       generate a sparse n-vector (v, iv)
c       having nzv nonzeros
c
c       mark(i) is set to 1 if position i is nonzero.
c       mark is all zero on entry and is reset to all zero before exit
c       this corrects a performance bug found by John G. Lewis, caused by
c       reinitialization of mark on every one of the n calls to sprnvc
---------------------------------------------------------------------*/
static void sprnvc(
    int n,
    int nz,
    double v[],
    int iv[],
    int nzloc[],	/* nzloc[1:n] */
    int mark[] ) 	/* mark[1:n] */
{
    int nn1;
    int nzrow, nzv, ii, i;
    double vecelt, vecloc;

    nzv = 0;
    nzrow = 0;
    nn1 = 1;
    do {
    	nn1 = 2 * nn1;
    } while (nn1 < n);

/*--------------------------------------------------------------------
c    nn1 is the smallest power of two not less than n
c-------------------------------------------------------------------*/

    while (nzv < nz) {
    	vecelt = randlc(&tran, amult);

/*--------------------------------------------------------------------
c   generate an integer between 1 and n in a portable manner
c-------------------------------------------------------------------*/
    	vecloc = randlc(&tran, amult);
    	i = icnvrt(vecloc, nn1) + 1;
    	if (i > n) continue;

/*--------------------------------------------------------------------
c  was this integer generated already?
c-------------------------------------------------------------------*/
    	if (mark[i] == 0) {
    	    mark[i] = 1;
    	    nzrow++;
    	    nzloc[nzrow] = i;
    	    v[nzv] = vecelt;
    	    iv[nzv] = i;
    	    nzv++;
    	}
    }

    for (ii = 1; ii <= nzrow; ii++) {
	    i = nzloc[ii];
	    mark[i] = 0;
    }
}

/*---------------------------------------------------------------------
* scale a double precision number x in (0,1) by a power of 2 and chop it
*---------------------------------------------------------------------*/
static int icnvrt(double x, int ipwr2) {
    return ((int)(ipwr2 * x));
}

/*--------------------------------------------------------------------
c       set ith element of sparse vector (v, iv) with
c       nzv nonzeros to val
c-------------------------------------------------------------------*/
static void vecset(
    double v[],	/* v[1:*] */
    int iv[],	/* iv[1:*] */
    int *nzv,
    int i,
    double val)
{
    int k;
    bool set;

    set = false;
    for (k = 0; k < *nzv; k++) {
    	if (iv[k] == i) {
                v[k] = val;
                set  = true;
    	}
    }
    if (set == false) {
	    v[*nzv] = val;
	    iv[*nzv] = i;
	    *nzv = *nzv + 1;
    }
}

