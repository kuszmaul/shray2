use IO;
use BlockDist;
use Time;

config const fileName = "cage11.mtx";
config const iterations = 10;

record CSRMatrix {
  /* The matrix is m x n */
  var m: int;
  var n: int;
  /* Number of non-zeroes */
  var nz: int;
  /* Stores the non-zeroes */
  var val: [1..nz] real;
  /* The first non-zero of row k is the row_ptr(k)th non-zero overall.
   * row_ptr(m + 1) is the total number of non-zeroes. */
  var row_ptr: [1..m + 1] int;
  /* The kth non-zero has column-index col_ind(k) */
  var col_ind: [1..nz] int;
}

/* mat is and output are seen as local by Chapel, vec is seen as distributed (blockwise).
 * I got errors when trying to pass the matrix as a single record. */
proc spmv(m: int, n: int, nz: int, val: [1..nz] real, row_ptr: [1..m + 1] int,
    col_ind: [1..nz] int, vec: [1..n] real)
{
  var result: [1..m] real = 0.0;

  forall row in 1..m {
    for j in row_ptr(row)..row_ptr(row + 1) - 1 {
      result(row) += val(j) * vec(col_ind(j));
    }
  }

  return result;
}

proc steady_state(m: int, n: int, nz: int, val: [1..nz] real, row_ptr: [1..m + 1] int,
    col_ind: [1..nz] int, iterations: int)
{
  const Space = {1..n};
  const BlockSpace = Space dmapped Block(boundingBox=Space);
  var blockSize: int = (n + numLocales - 1) / numLocales;

  var vec: [BlockSpace] real = 1 / n;

  sync for t in 1..iterations do
    /* As a stochastic matrix is square and we distributed the matrix blockwise along the first
     * dimension, mat.m is precisely the size of the local part of the vector. */
    vec(here.id * blockSize + 1 .. here.id * blockSize + m) =
      spmv(m, n, nz, val, row_ptr, col_ind, vec);

  return vec;
}

proc main()
{
  /******************************************************************
   * IO handling
   *****************************************************************/
  var mat: CSRMatrix;

  var infoName = fileName + "_info" + (here.id + 1) : string;
  var infoFile = open(infoName, ioMode.r);
  var infoChannel = infoFile.reader();
  var m: int;
  var n: int;
  var nz: int;
  var allNzs: [1..numLocales] int;

  if (!infoChannel.read(m)) then
    writeln("IO error\n");

  if (!infoChannel.read(n)) then
    writeln("IO error\n");

  if (!infoChannel.read(nz)) then
    writeln("IO error\n");

  infoChannel.close();
  mat.m = m;
  mat.n = n;
  mat.nz = nz;
  allNzs[here.id + 1] = nz;


  var valuesName = fileName + "_values" + (here.id + 1) : string;
  var valuesFile = open(valuesName, ioMode.r);
  var valuesChannel = valuesFile.reader();
  var val: [1..nz] real;

  for i in 1..mat.nz do
    if(!valuesChannel.read(val(i))) then
      writeln("IO error\n");

  valuesChannel.close();
  mat.val = val;


  var rowName = fileName + "_row" + (here.id + 1) : string;
  var rowFile = open(rowName, ioMode.r);
  var rowChannel = rowFile.reader();
  var row_ptr: [1..m + 1] int;

  for i in 1..mat.m + 1 do
    if (!rowChannel.read(row_ptr(i))) then
      writeln("IO error\n");

  rowChannel.close();
  mat.row_ptr = row_ptr;


  var columnName = fileName + "_column" + (here.id + 1) : string;
  var columnFile = open(columnName, ioMode.r);
  var columnChannel = columnFile.reader();
  var col_ind: [1..nz] int;

  for i in 1..mat.nz do
    if(!columnChannel.read(col_ind(i))) then
      writeln("IO error\n");

  columnChannel.close();
  mat.col_ind = col_ind;

  /******************************************************************
   * The main program. Each locale reads in part of the matrix, block
   * distributed along the rows, as just a normal local variable.
   * The vector we multiply with is block distributed and actually
   * a distributed variable.
   *****************************************************************/

  const Space = {1..mat.n};
  const BlockSpace = Space dmapped Block(boundingBox=Space);
  var vec: [BlockSpace] real = 1 / mat.n;

  var watch: stopwatch;
  watch.start();

  coforall loc in Locales do on loc {
    vec = steady_state(m, n, nz, val, row_ptr, col_ind, iterations);
  }

  watch.stop();

  if (here.id == 0) then
    stdout.writeln(2.0 * (+ reduce allNzs) * iterations / 1000000000.0 / watch.elapsed(), '\n');
}
