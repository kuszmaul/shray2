/* Assumes n is divisible by number of locales * number of threads per locale.
 * The latter can be set with CHPL_RT_NUM_THREADS_PER_LOCALE */


use Time;
use IO;
use BLAS;
use BlockDist;
/* Needs BLAS or lapack */
use LinearAlgebra;
use AllLocalesBarriers;

config const n: int = 4000;
config const numTasks = 2;

var MyLocaleView = {0..#numLocales, 1..1};
var MyLocales: [MyLocaleView] locale = reshape(Locales, MyLocaleView);

const Space = {1..n, 1..n};
const BlockSpace = Space dmapped Block(boundingBox=Space,
                                        targetLocales=MyLocales);

var A: [BlockSpace] real = 1;
var B: [BlockSpace] real = 1;
var C: [BlockSpace] real;

var watch: Timer;
watch.start();

proc parallel_mul(As: [1..n / numLocales, 1..n / numLocales] real, Bl: [1..n / numLocales,1..n] real)
{
  var C: [1..n / numLocales,1..n] real;
  var Cpart: [1..n / numLocales / numTasks,1..n] real;
  var Apart: [1..n / numLocales / numTasks,1..n / numLocales] real;

  coforall t in 1..numTasks {
    stdout.writeln("Task ", t);
    Apart = As(1 + n / numLocales / numTasks * (t - 1) .. 1 + n / numLocales / numTasks, 1..n / numLocales);
    Cpart = dot(As, B);
    C[1 + n / numLocales / numTasks * (t - 1) .. 1 + n / numLocales / numTasks, 1..n] = Cpart;
  }

  return C;
}

coforall loc in Locales do on loc {
  for l in 0..numLocales - 1 {
        /* We have to copy out As, Bl to avoid segfaults, probably because dot uses
         * an external library. */
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
