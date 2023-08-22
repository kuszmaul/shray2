/***********************************************************************
 * This is a version that uses multilocale exeuction for parallelism
 * between nodes, and multitask exeuction for parallelism within a node.
 *
 * We calculate AB by
 *
 *   A0      A0B
 *  ...  B = ...
 *   Ap      ApB
 *
 * and allocating As only on node s. The same trick is then applied
 * with tasks to calculate AsB.
 ***********************************************************************/

use Time;
use IO;
use BLAS;
use BlockDist;
/* Needs BLAS or lapack */
use LinearAlgebra;
use AllLocalesBarriers;

config const n: int = 4000;

var MyLocaleView = {0..#numLocales, 1..1};
var MyLocales: [MyLocaleView] locale = reshape(Locales, MyLocaleView);

const Space = {1..n, 1..n};
const BlockSpace = Space dmapped Block(boundingBox=Space,
                                        targetLocales=MyLocales);

var A: [BlockSpace] real = 1;
var B: [BlockSpace] real = 1;
var C: [BlockSpace] real;

var watch: stopwatch;
watch.start();

/* cotasks, assumes p | n */
proc matmul(n: int, A: [0..n - 1, 0..n - 1] real, B: [0..n - 1, 0..n - 1] real)
{

}

coforall loc in Locales do on loc {
  for l in 0..numLocales - 1 {
        /* We have to copy out As, Bl to avoid segfaults, probably because dot uses
         * an external library. *
        var As: [1..n / numLocales,1..n / numLocales] real =
          A[A.localSubdomain()](.., 1 + l * n / numLocales..(l + 1) * n / numLocales);
        var Bl: [1..n / numLocales,1..n] real = B[B.localSubdomain(Locales[l])];
        C[C.localSubdomain()] += dot(As, Bl);
        writeln("Locale ", here.id, " has done loop iteration ", l, "\n");
  }

  allLocalesBarrier.barrier();

  for (i, j) in {1..n, 1..n} {
    if (C[i, j] != n) {
      stdout.writeln("Index (", i, ", ", j, ") should be ", n, ", but is ", C[i, j], "\n");
    }
  }
}

watch.stop();

if (here.id == 0) then
  stdout.writeln(2.0 * n * n * n / watch.elapsed() / 1000000000, '\n');
