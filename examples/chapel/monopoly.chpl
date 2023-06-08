use IO;
use BlockDist;
use Time;

config const n = 100;
config const iterations = 1;

/* mat is and output are seen as local by Chapel, vec is seen as distributed (blockwise).
 * I got errors when trying to pass the matrix as a single record. */
proc spmv(m: int, n: int, nz: int, val: [0..nz - 1] real, row_ptr: [0..m] int,
    col_ind: [0..nz - 1] int, vec: [0..n - 1] real)
{
  var result: [0..m - 1] real = 0.0;

  for row in 0..m - 1 {
    for j in row_ptr(row)..row_ptr(row + 1) - 1 {
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
    vec(here.id * blockSize .. here.id * blockSize + m - 1) =
      spmv(m, n, nz, val, row_ptr, col_ind, vec);

  return vec;
}

proc main()
{
  var m: int;

  var blockSize: int = (n + numLocales - 1) / numLocales;
  if (here.id == numLocales - 1) {
    m = n - (numLocales - 1) * blockSize;
  } else {
    m = blockSize;
  }
  var nz: int = 11 * m;

  var val: [0..nz - 1] real;
  var row_ptr: [0..m] int;
  var col_ind: [0..nz - 1] int;

  var probs: [2..12] real = [1 / 36, 2 / 36, 3 / 36, 4 / 36, 5 / 36, 6 / 36,
                                 5 / 36, 4 / 36, 3 / 36, 2 / 36, 1 / 36];

  var i: int = 0;
  for row in {0..m - 1} do {
    var global_row: int = here.id * blockSize + row;
    for eyes in {2..12} do {
      val(i) = probs(eyes);
      col_ind(i) = (global_row + eyes) % n;
      i = i + 1;
    }
    row_ptr(row) = 11 * row;
  }
  row_ptr(m) = 11 * m;

  /******************************************************************
   * The main program. Each locale reads in part of the matrix, block
   * distributed along the rows, as just a normal local variable.
   * The vector we multiply with is block distributed and actually
   * a distributed variable.
   *****************************************************************/

  const Space = {0..n - 1};
  const BlockSpace = Space dmapped Block(boundingBox=Space);
  var vec: [BlockSpace] real = 1 / n;

  var watch: stopwatch;
  watch.start();

  coforall loc in Locales do on loc {
    vec = steady_state(m, n, nz, val, row_ptr, col_ind, iterations);
  }

  watch.stop();

  if (here.id == 0) then
    stdout.writeln(2.0 * 11.0 * n * iterations / 1000000000.0 / watch.elapsed(), '\n');
}
