use IO;
use BlockDist;
use Time;

config const n = 100;
config const iterations = 1;

record CSRMatrix {
  /* The matrix is m x n */
  var m: int;
  var n: int;
  /* Number of non-zeroes */
  var nz: int;
  /* Stores the non-zeroes */
  var val: [0..nz - 1] real;
  /* The first non-zero of row k is the row_ptr(k)th non-zero overall.
   * row_ptr(m + 1) is the total number of non-zeroes. */
  var row_ptr: [0..m] int;
  /* The kth non-zero has column-index col_ind(k) */
  var col_ind: [0..nz - 1] int;
}

/* mat is and output are seen as local by Chapel, vec is seen as distributed (blockwise).
 * I got errors when trying to pass the matrix as a single record. */
proc spmv(m: int, n: int, nz: int, val: [0..nz - 1] real, row_ptr: [0..m] int,
    col_ind: [0..nz - 1] int, vec: [0..n - 1] real)
{
  var result: [0..m - 1] real = 0.0;

  for row in 0..m - 1 {
    for j in row_ptr(row)..row_ptr(row + 1) - 1 {
      stdout.writeln("j is ", j);
      stdout.writeln("col_ind(j) is ");
      stdout.writeln(col_ind(j));
      result(row) += val(j) * vec(col_ind(j));
    }
  }

  return result;
}

proc steady_state(m: int, n: int, nz: int, val: [0..nz - 1] real, row_ptr: [0..m] int,
    col_ind: [0..nz - 1] int, iterations: int)
{
  const Space = {0..n - 1};
  const BlockSpace = Space dmapped Block(boundingBox=Space);
  var blockSize: int = (n + numLocales - 1) / numLocales;

  var vec: [BlockSpace] real = 1 / n;

  sync for t in 1..iterations do
    /* As a stochastic matrix is square and we distributed the matrix blockwise along the first
     * dimension, mat.m is precisely the size of the local part of the vector. */
    stdout.writeln(t, " iteration");
    vec(here.id * blockSize .. here.id * blockSize + m - 1) =
      spmv(m, n, nz, val, row_ptr, col_ind, vec);

  return vec;
}

proc main()
{
  var mat: CSRMatrix;

  var blockSize: int = (n + numLocales - 1) / numLocales;
  if (here.id == numLocales - 1) {
    mat.m = n - (numLocales - 1) * blockSize;
  } else {
    mat.m = blockSize;
  }
  mat.n = n;
  mat.nz = 11 * mat.m;

  var val: [0..mat.nz - 1] real;
  var row_ptr: [0..mat.m] int;
  var col_ind: [0..mat.nz - 1] int;

  var probs: [2..12] real = [1 / 36, 2 / 36, 3 / 36, 4 / 36, 5 / 36, 6 / 36,
                                 5 / 36, 4 / 36, 3 / 36, 2 / 36, 1 / 36];

  var i: int = 0;
  for row in {0..mat.m - 1} do {
    var global_row: int = here.id * blockSize + row;
    for eyes in {2..12} do {
      val(i) = probs(eyes);
      col_ind(i) = (global_row + eyes) % n;
      stdout.writeln("col_ind(", i, ") is ", col_ind(i));
      i = i + 1;
    }
    row_ptr(row) = 11 * row;
  }
  row_ptr(mat.m) = 11 * mat.m;

  mat.val = val;
  mat.row_ptr = row_ptr;
  mat.col_ind = col_ind;


  /******************************************************************
   * The main program. Each locale reads in part of the matrix, block
   * distributed along the rows, as just a normal local variable.
   * The vector we multiply with is block distributed and actually
   * a distributed variable.
   *****************************************************************/

  const Space = {0..mat.n - 1};
  const BlockSpace = Space dmapped Block(boundingBox=Space);
  var vec: [BlockSpace] real = 1 / mat.n;

  var watch: Timer;
  watch.start();

  stdout.writeln("Start computation");
  vec = steady_state(mat.m, mat.n, mat.nz, mat.val, mat.row_ptr, mat.col_ind, iterations);

  watch.stop();

  if (here.id == 0) then
    stdout.writeln(2.0 * 11.0 * n * iterations / 1000000000.0 / watch.elapsed(), '\n');
}
