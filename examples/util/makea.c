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
static void makea(int n, size_t nz, double a[], long colidx[], long rowstr[],
		  int nonzer, double rcond, int arow[], int acol[],
		  double aelt[], double v[], int iv[], double shift );
static void sparse(double a[], long colidx[], long rowstr[], int n,
		   int arow[], int acol[], double aelt[],
		   double x[], int mark[], int nzloc[], size_t nnza);
static void sprnvc(int n, size_t nz, double v[], int iv[], size_t nzloc[],
		   size_t mark[]);
static int icnvrt(double x, int ipwr2);
static void vecset(double v[], int iv[], int *nzv, int i, double val);

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

    long NA, NONZER, NITER;
    double RCOND = 1.0e-1;
    double SHIFT;

    if (!strcmp(class, "S")) {
        NA = 1400;
        NONZER = 7;
        NITER = 15;
        SHIFT = 10.0;
    } else if (!strcmp(class, "W")) {
        NA = 7000;
        NONZER = 8;
        NITER = 15;
        SHIFT = 12.0;
    } else if (!strcmp(class, "A")) {
        NA = 14000;
        NONZER = 11;
        NITER = 15;
        SHIFT = 20.0;
    } else if (!strcmp(class, "B")) {
        NA = 75000;
        NONZER = 13;
        NITER = 75;
        SHIFT = 60.0;
    } else if (!strcmp(class, "C")) {
        NA = 150000;
        NONZER = 15;
        NITER = 75;
        SHIFT = 110.0;
    } else if (!strcmp(class, "D")) {
        NA = 1500000;
        NONZER = 21;
        NITER = 100;
        SHIFT = 500.0;
    } else if (!strcmp(class, "E")) {
        NA = 9000000;
        NONZER = 26;
        NITER = 100;
        SHIFT = 1.5;
    }else {
        fprintf(stderr, "Error, class should be in {S, W, A, B, C}\n");
        return EXIT_FAILURE;
    }

    /* For the largest problem sizes, NA can be held by an integer, but not NZ
     * for class >= E. */
    long NZ = NA * (NONZER + 1) * (NONZER + 1) + NA * (NONZER + 2);

    fprintf(stderr, "\n\n NAS Parallel Benchmarks 3.0 structured OpenMP C version"
	   " - CG Benchmark\n");
    fprintf(stderr, " Size: %10ld\n", NA);
    fprintf(stderr, " Iterations: %5ld\n", NITER);

    int naa = NA;
    long nzz = NZ;

/*--------------------------------------------------------------------
c  Initialize random number generator and generate one so a is
c  generated with the right seed.
c-------------------------------------------------------------------*/
    tran    = 314159265.0;
    amult   = 1220703125.0;
    randlc( &tran, amult );

/*--------------------------------------------------------------------
c
c-------------------------------------------------------------------*/

    long *colidx = malloc(NZ * sizeof(long));
    long *rowstr = malloc((NA + 1) * sizeof(long));
    long *colidx2 = malloc(NZ * sizeof(long));
    long *rowstr2 = malloc((NA + 1) * sizeof(long));
    int *iv = malloc(2 * NA * sizeof(int));
    int *arow = malloc(NZ * sizeof(int));
    int *acol = malloc(NZ * sizeof(int));

    double *v = malloc((NA + 1) * sizeof(double));
    double *aelt = malloc(NZ * sizeof(double));
    double *a = malloc(NZ * sizeof(double));
    double *a2 = malloc(NZ * sizeof(double));

    makea(naa, nzz, a, colidx, rowstr, NONZER,
	  RCOND, arow, acol, aelt, v, iv, SHIFT);

    fprintf(stderr, "First 10 values of a\n");
    for (int i = 0; i < 10; i++) {
        fprintf(stderr, "%lf\n", a[i]);
    }

    fprintf(stderr, "First 10 values of colidx\n");
    for (int i = 0; i < 10; i++) {
        fprintf(stderr, "%ld\n", colidx[i]);
    }

    fprintf(stderr, "First 10 values of rowstr\n");
    for (int i = 0; i < 10; i++) {
        fprintf(stderr, "%ld\n", rowstr[i]);
    }

    char *a_name = malloc(7);
    sprintf(a_name, "a.cg.%s", class);
    char *colidx_name = malloc(12);
    sprintf(colidx_name, "colidx.cg.%s", class);
    char *rowstr_name = malloc(12);
    sprintf(rowstr_name, "rowstr.cg.%s", class);

    FILE *a_stream = fopen(a_name, "w");
    FILE *colidx_stream = fopen(colidx_name, "w");
    FILE *rowstr_stream = fopen(rowstr_name, "w");

    if (!a_stream || !colidx_stream || !rowstr_stream) {
        printf("Opening files failed\n");
        return EXIT_FAILURE;
    }

    size_t bytes_written;
    if ((bytes_written = fwrite(a, sizeof(double), NZ, a_stream)) != NZ) {
        fprintf(stderr, "Writing %s went wrong, only wrote %lu items\n",
                a_name, bytes_written);
        return EXIT_FAILURE;
    }
    if ((bytes_written = fwrite(colidx, sizeof(long), NZ, colidx_stream))
            != NZ) {
        fprintf(stderr, "Writing %s went wrong\n", colidx_name);
        return EXIT_FAILURE;
    }
    if ((bytes_written = fwrite(rowstr, sizeof(long), NA + 1, rowstr_stream))
            != NA + 1) {
        fprintf(stderr, "Writing %s went wrong\n", rowstr_name);
        return EXIT_FAILURE;
    }

    fclose(a_stream);
    fclose(colidx_stream);
    fclose(rowstr_stream);

    char name[50];
    sprintf(name, "a.cg.%s", class);
    read_sparse(name, a2, NZ * sizeof(double));
    sprintf(name, "colidx.cg.%s", class);
    read_sparse(name, colidx2, NZ * sizeof(long));
    sprintf(name, "rowstr.cg.%s", class);
    read_sparse(name, rowstr2, (NA + 1) * sizeof(long));

    bool correct_a = true;
    for (size_t i = 0; i < NZ; i++) {
        if (a2[i] != a[i]) {
            correct_a = false;
            printf("Created a[%ld] = %.17lf is wrong (should be %.17lf)\n",
                    i, a2[i], a[i]);
        }
    }

    fprintf(stderr, "Checked output of a, is %scorrect!\n",
            (correct_a) ? "" : "in");

    bool correct_col = true;
    for (size_t i = 0; i < NZ; i++) {
        if (colidx2[i] != colidx[i]) {
            correct_col = false;
            printf("Created colidx[%ld] = %ld is wrong (should be %ld)\n",
                    i, colidx2[i], colidx[i]);
        }
    }

    fprintf(stderr, "Checked output of colidx, is %scorrect!\n",
            (correct_col) ? "" : "in");

    bool correct_row = true;
    for (int i = 0; i < NA + 1; i++) {
        if (rowstr2[i] != rowstr[i]) {
            correct_row = false;
            printf("Created rowstr[%d] = %ld is wrong (should be %ld)\n",
                    i, rowstr2[i], rowstr[i]);
        }
    }

    fprintf(stderr, "Checked output of rowstr, is %scorrect!\n",
            (correct_row) ? "" : "in");

    free(colidx);
    free(rowstr);
    free(iv);
    free(arow);
    free(acol);
    free(aelt);
    free(a);
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
c       nz           i*8         number of non-zeroes of matrix
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
    size_t nz,
    double a[],
    long colidx[],
    long rowstr[],
    int nonzer,
    double rcond,
    int arow[],
    int acol[],
    double aelt[],
    double v[],
    int iv[],
    double shift )
{
    int iouter, ivelt, ivelt1, irow, nzv;
    size_t i, nnza;

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
    for (i = 0; i < (size_t)n; i++) {
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
    for (i = 1; i <= (size_t)n; i++) {
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

    /* Ugly but we should index the non-zeroes from 0 to n - 1, not 1 to n. */
    #pragma omp parallel for private(i)
    for (i = 0; i < nz; i++) {
        colidx[i]--;
    }
}

/*---------------------------------------------------
c       generate a sparse matrix from a list of
c       [col, row, element] tri
c---------------------------------------------------*/
static void sparse(
    double a[],
    long colidx[],
    long rowstr[],
    int n,
    int arow[],
    int acol[],
    double aelt[],
    double x[],
    int mark[],	/* mark[1:n] */
    int nzloc[],	/* nzloc[1:n] */
    size_t nnza)
/*---------------------------------------------------------------------
c       rows range from 1 to n
c       the rowstr pointers are defined for nrows = n values
c---------------------------------------------------------------------*/
{
    int nrows;
    size_t i, j, jajp1, nza, k, nzrow;
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

    rowstr[0] = 0;
    for (j = 1; j <= (size_t)nrows; j++) {
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
	       a[k] = 0.0;
      }
/*--------------------------------------------------------------------
c     ... do a bucket sort of the triples on the row index
c-------------------------------------------------------------------*/
    for (nza = 0; nza < nnza; nza++) {
	    j = arow[nza] - 1;
	    k = rowstr[j];
	    a[k] = aelt[nza];
	    colidx[k] = acol[nza];
	    rowstr[j] = rowstr[j] + 1;
    }

/*--------------------------------------------------------------------
c       ... rowstr(j) now points to the first element of row j+1
c-------------------------------------------------------------------*/
    for (j = nrows; j >= 1; j--) {
	    rowstr[j] = rowstr[j - 1];
    }
    rowstr[0] = 0;

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
    for (j = 1; j <= (size_t)nrows; j++) {
	    nzrow = 0;

/*--------------------------------------------------------------------
c              ...loop over the jth row of a
c-------------------------------------------------------------------*/
	    for (k = jajp1; k < rowstr[j]; k++) {
            i = colidx[k];
            x[i] += a[k];
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
    size_t nz,
    double v[],
    int iv[],
    size_t nzloc[],	/* nzloc[1:n] */
    size_t mark[] ) 	/* mark[1:n] */
{
    int nn1;
    size_t nzrow, nzv, ii, i;
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
    	if (i > (size_t)n) continue;

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

