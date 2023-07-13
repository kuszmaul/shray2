/*--------------------------------------------------------------------

  Authors: M. Yarrow
           C. Kuszmaul

  OpenMP C version: S. Satoh

  3.0 structure translation: F. Conti

  Modification for outputting a by T. Koopman

--------------------------------------------------------------------*/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <stdbool.h>
#include <unistd.h>
#include <omp.h>
#include <string.h>

/* For random numbers */
#define r23 pow(0.5, 23.0)
#define r46 (r23*r23)
#define t23 pow(2.0, 23.0)
#define t46 (t23*t23)

/* global variables */

char *class;

/* common /partit_size/ */
static int naa;
static long nzz;
static int firstrow;
static int lastrow;
static int firstcol;
static int lastcol;

/* common /urando/ */
static double amult;
static double tran;

/* function declarations */
static void makea(int n, int nz, double a[], int colidx[], int rowstr[],
		  int nonzer, int firstrow, int lastrow, int firstcol,
		  int lastcol, double rcond, int arow[], int acol[],
		  double aelt[], double v[], int iv[], double shift );
static void sparse(double a[], int colidx[], int rowstr[], int n,
		   int arow[], int acol[], double aelt[],
		   int firstrow, int lastrow,
		   double x[], int mark[], int nzloc[], int nnza);
static void sprnvc(int n, int nz, double v[], int iv[], int nzloc[],
		   int mark[]);
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
        fprintf(stderr, "Usage: %s class with class in {S, W, A, B, C}\n"
                        "Will output a.cg.class for the non-zeroes\n"
                        "colidx.cg.class for the column incides\n"
                        "rowstr.cg.class for the row pointers\n",
                        argv[0]);
        return EXIT_FAILURE;
    }

    class = argv[1];

    int NA, NONZER, NITER;
    double RCOND = 1.0e-1;
    double SHIFT;

    if (!strcmp(class, "S")) {
        NA = 1400;
        NONZER = 7;
        NITER = 15;
        SHIFT = 10.0;
    } else if (!strcmp(class, "W")) {
        NA = 7000;
        NONZER = 7;
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
    } else {
        fprintf(stderr, "Error, class should be in {S, W, A, B, C}\n");
        return EXIT_FAILURE;
    }

    fprintf(stderr, "Generating file for program class %s\n", class);

    long NZ = NA * (NONZER + 1) * (NONZER + 1) + NA * (NONZER + 2);

    firstrow = 1;
    lastrow  = NA;
    firstcol = 1;
    lastcol  = NA;

    naa = NA;
    nzz = NZ;

    int *colidx = malloc((NZ+1) * sizeof(int));	/* colidx[1:NZ] */
    int *colidx2 = malloc((NZ+1) * sizeof(int));	/* colidx[1:NZ] */
    int *rowstr = malloc((NA+1+1) * sizeof(int));	/* rowstr[1:NA+1] */
    int *rowstr2 = malloc((NA+1+1) * sizeof(int));	/* rowstr[1:NA+1] */
    int *iv = malloc((2*NA+1+1) * sizeof(int));	/* iv[1:2*NA+1] */
    int *arow = malloc((NZ+1) * sizeof(int));		/* arow[1:NZ] */
    int *acol = malloc((NZ+1) * sizeof(int));		/* acol[1:NZ] */

    double *v = malloc((NA+1+1) * sizeof(double));	/* v[1:NA+1] */
    double *aelt = malloc((NZ+1) * sizeof(double));	/* aelt[1:NZ] */
    double *a = malloc((NZ+1) * sizeof(double));		/* a[1:NZ] */
    double *a2 = malloc((NZ+1) * sizeof(double));		/* a[1:NZ] */


/*--------------------------------------------------------------------
c  Initialize random number generator
c-------------------------------------------------------------------*/
    tran    = 314159265.0;
    amult   = 1220703125.0;

/*--------------------------------------------------------------------
c
c-------------------------------------------------------------------*/
    makea(naa, nzz, a, colidx, rowstr, NONZER,
	  firstrow, lastrow, firstcol, lastcol,
	  RCOND, arow, acol, aelt, v, iv, SHIFT);

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
    if ((fwrite(a, sizeof(double), NZ + 1, a_stream) != NZ + 1)) {
        fprintf(stderr, "Writing %s went wrong, only wrote %lu items\n",
                a_name, bytes_written);
        return EXIT_FAILURE;
    }
    if ((fwrite(colidx, sizeof(int), NZ + 1, colidx_stream) != NZ + 1)) {
        fprintf(stderr, "Writing %s went wrong\n", colidx_name);
        return EXIT_FAILURE;
    }
    if ((fwrite(rowstr, sizeof(int), NA + 2, rowstr_stream) != NA + 2)) {
        fprintf(stderr, "Writing %s went wrong\n", rowstr_name);
        return EXIT_FAILURE;
    }

    fclose(a_stream);
    fclose(colidx_stream);
    fclose(rowstr_stream);

    char name[50];
    sprintf(name, "a.cg.%s", class);
    read_sparse(name, a2, (NZ + 1) * sizeof(double));
    sprintf(name, "colidx.cg.%s", class);
    read_sparse(name, colidx2, (NZ + 1) * sizeof(int));
    sprintf(name, "rowstr.cg.%s", class);
    read_sparse(name, rowstr2, (NA + 1 + 1) * sizeof(int));

    bool correct_a = true;
    for (int i = 0; i < NZ + 1; i++) {
        if (a2[i] != a[i]) {
            correct_a = false;
            printf("Created a[%d] = %.17lf is wrong (should be %.17lf)\n",
                    i, a2[i], a[i]);
        }
    }

    fprintf(stderr, "Checked output of a, is %scorrect!\n",
            (correct_a) ? "" : "in");

    bool correct_col = true;
    for (int i = 0; i < NZ + 1; i++) {
        if (colidx2[i] != colidx[i]) {
            correct_col = false;
            printf("Created colidx[%d] = %d is wrong (should be %d)\n",
                    i, colidx2[i], colidx[i]);
        }
    }

    fprintf(stderr, "Checked output of colidx, is %scorrect!\n",
            (correct_col) ? "" : "in");

    bool correct_row = true;
    for (int i = 0; i < NA + 1 + 1; i++) {
        if (rowstr2[i] != rowstr[i]) {
            correct_row = false;
            printf("Created rowstr[%d] = %d is wrong (should be %d)\n",
                    i, rowstr2[i], rowstr[i]);
        }
    }

    fprintf(stderr, "Checked output of rowstr, is %scorrect!\n",
            (correct_row) ? "" : "in");

    free(colidx);
    free(colidx2);
    free(rowstr);
    free(rowstr2);
    free(iv);
    free(arow);
    free(acol);
    free(v);
    free(aelt);
    free(a);
    free(a2);

    return EXIT_SUCCESS;
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
c       nz           i           nonzeros as declared array size
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
    int nz,
    double a[],		/* a[1:nz] */
    int colidx[],	/* colidx[1:nz] */
    int rowstr[],	/* rowstr[1:n+1] */
    int nonzer,
    int firstrow,
    int lastrow,
    int firstcol,
    int lastcol,
    double rcond,
    int arow[],		/* arow[1:nz] */
    int acol[],		/* acol[1:nz] */
    double aelt[],	/* aelt[1:nz] */
    double v[],		/* v[1:n+1] */
    int iv[],		/* iv[1:2*n+1] */
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
c  Initialize colidx(n+1 .. 2n) to zero.
c  Used by sprnvc to mark nonzero positions
c---------------------------------------------------------------------*/
#pragma omp parallel for default(shared) private(i)
    for (i = 1; i <= n; i++) {
    	colidx[n+i] = 0;
    }
    for (iouter = 1; iouter <= n; iouter++) {
	    nzv = nonzer;
	    sprnvc(n, nzv, v, iv, &(colidx[0]), &(colidx[n]));
	    vecset(v, iv, &nzv, iouter, 0.5);
	    for (ivelt = 1; ivelt <= nzv; ivelt++) {
	        jcol = iv[ivelt];
	        if (jcol >= firstcol && jcol <= lastcol) {
	    	    scale = size * v[ivelt];
	    	    for (ivelt1 = 1; ivelt1 <= nzv; ivelt1++) {
	                irow = iv[ivelt1];
                    if (irow >= firstrow && irow <= lastrow) {
	    	    	    nnza = nnza + 1;
	    	    	    if (nnza > nz) {
	    	    	        printf("Space for matrix elements exceeded in"
	    	    	    	   " makea\n");
	    	    	        printf("nnza, nzmax = %d, %d\n", nnza, nz);
	    	    	        printf("iouter = %d\n", iouter);
	    	    	        exit(1);
	    	    	    }
	    	    	    acol[nnza] = jcol;
	    	    	    arow[nnza] = irow;
	    	    	    aelt[nnza] = v[ivelt1] * scale;
	    	        }
	    	    }
	        }
	    }
	    size = size * ratio;
    }

/*---------------------------------------------------------------------
c       ... add the identity * rcond to the generated matrix to bound
c           the smallest eigenvalue from below by rcond
c---------------------------------------------------------------------*/
    for (i = firstrow; i <= lastrow; i++) {
    	if (i >= firstcol && i <= lastcol) {
    	    iouter = n + i;
    	    nnza = nnza + 1;
    	    if (nnza > nz) {
    		    printf("Space for matrix elements exceeded in makea\n");
    		    printf("nnza, nzmax = %d, %d\n", nnza, nz);
    		    printf("iouter = %d\n", iouter);
    		    exit(1);
    	    }
    	    acol[nnza] = i;
    	    arow[nnza] = i;
    	    aelt[nnza] = rcond - shift;
    	}
    }

/*---------------------------------------------------------------------
c       ... make the sparse matrix from list of elements with duplicates
c           (v and iv are used as  workspace)
c---------------------------------------------------------------------*/
    sparse(a, colidx, rowstr, n, arow, acol, aelt,
	   firstrow, lastrow, v, &(iv[0]), &(iv[n]), nnza);
}

/*---------------------------------------------------
c       generate a sparse matrix from a list of
c       [col, row, element] tri
c---------------------------------------------------*/
static void sparse(
    double a[],		/* a[1:*] */
    int colidx[],	/* colidx[1:*] */
    int rowstr[],	/* rowstr[1:*] */
    int n,
    int arow[],		/* arow[1:*] */
    int acol[],		/* acol[1:*] */
    double aelt[],	/* aelt[1:*] */
    int firstrow,
    int lastrow,
    double x[],		/* x[1:n] */
    int mark[],	/* mark[1:n] */
    int nzloc[],	/* nzloc[1:n] */
    int nnza)
/*---------------------------------------------------------------------
c       rows range from firstrow to lastrow
c       the rowstr pointers are defined for nrows = lastrow-firstrow+1 values
c---------------------------------------------------------------------*/
{
    int nrows;
    int i, j, jajp1, nza, k, nzrow;
    double xi;

/*--------------------------------------------------------------------
c    how many rows of result
c-------------------------------------------------------------------*/
    nrows = lastrow - firstrow + 1;

/*--------------------------------------------------------------------
c     ...count the number of triples in each row
c-------------------------------------------------------------------*/
#pragma omp parallel for default(shared) private(j)
    for (j = 1; j <= n; j++) {
	    rowstr[j] = 0;
	    mark[j] = false;
    }
    rowstr[n+1] = 0;

    for (nza = 1; nza <= nnza; nza++) {
	    j = (arow[nza] - firstrow + 1) + 1;
	    rowstr[j] = rowstr[j] + 1;
    }

    rowstr[1] = 1;
    for (j = 2; j <= nrows+1; j++) {
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
      for(j = 0;j <= nrows-1;j++) {
         for(k = rowstr[j];k <= rowstr[j+1]-1;k++)
	       a[k] = 0.0;
      }
/*--------------------------------------------------------------------
c     ... do a bucket sort of the triples on the row index
c-------------------------------------------------------------------*/
    for (nza = 1; nza <= nnza; nza++) {
	    j = arow[nza] - firstrow + 1;
	    k = rowstr[j];
	    a[k] = aelt[nza];
	    colidx[k] = acol[nza];
	    rowstr[j] = rowstr[j] + 1;
    }

/*--------------------------------------------------------------------
c       ... rowstr(j) now points to the first element of row j+1
c-------------------------------------------------------------------*/
    for (j = nrows; j >= 1; j--) {
	    rowstr[j+1] = rowstr[j];
    }
    rowstr[1] = 1;

/*--------------------------------------------------------------------
c       ... generate the actual output rows by adding elements
c-------------------------------------------------------------------*/
    nza = 0;
#pragma omp parallel for default(shared) private(i)
    for (i = 1; i <= n; i++) {
	    x[i] = 0.0;
	    mark[i] = false;
    }

    jajp1 = rowstr[1];
    for (j = 1; j <= nrows; j++) {
	    nzrow = 0;

/*--------------------------------------------------------------------
c              ...loop over the jth row of a
c-------------------------------------------------------------------*/
	    for (k = jajp1; k < rowstr[j+1]; k++) {
            i = colidx[k];
            x[i] = x[i] + a[k];
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
	    	    nza = nza + 1;
	    	    a[nza] = xi;
	    	    colidx[nza] = i;
	        }
	    }
	    jajp1 = rowstr[j+1];
	    rowstr[j+1] = nza + rowstr[1];
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
    double v[],		/* v[1:*] */
    int iv[],		/* iv[1:*] */
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
	    nzrow = nzrow + 1;
	    nzloc[nzrow] = i;
	    nzv = nzv + 1;
	    v[nzv] = vecelt;
	    iv[nzv] = i;
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
    for (k = 1; k <= *nzv; k++) {
	if (iv[k] == i) {
            v[k] = val;
            set  = true;
	}
    }
    if (set == false) {
	*nzv = *nzv + 1;
	v[*nzv] = val;
	iv[*nzv] = i;
    }
}
