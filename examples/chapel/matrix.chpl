/* Assumes n is divisible by number of locales */

use Time;
use IO;
use BLAS;
use BlockDist;
/* Needs BLAS or lapack */
use LinearAlgebra;

config const n: int = 4000;

const Space = {1..n, 1..n};
const BlockSpace = Space dmapped Block(boundingBox=Space);
var A: [BlockSpace] real = 1;
var B: [BlockSpace] real = 1;
var C: [BlockSpace] real;

var watch: Timer;
watch.start();

for l in 1..numLocales {
    C[C.localSubdomain()] += dot(A[A.localSubdomain()](.., 1..n / numLocales),
        B[B.localSubdomain(Locales[l])]);
}

watch.stop();

if (here.id == 0) then
  stdout.writeln(2.0 * n * n * n / watch.elapsed() / 1000000000, '\n');
