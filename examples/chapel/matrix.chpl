use Time;
use IO;
use BLAS;
/* Needs BLAS or lapack */
use LinearAlgebra;

/* Use ./2dstencil --n=20000 */
config const n: int = 4000;

var mat: [1..n, 1..n] real = 1;
var watch: Timer;
watch.start();

mat = dot(mat, mat);

watch.stop();
stderr.writeln('Anti-optimisation number: ', + reduce mat, '\n');

stdout.writeln(2.0 * n * n * n / watch.elapsed() / 1000000000, '\n');
