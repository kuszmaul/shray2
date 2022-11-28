use Time;
use IO;

/* Use ./2dstencil --n=20000 --iterations=5 to run with different arguments. */
config const n: int = 10000;
config const iterations: int = 10;

const constants: [1..3, 1..3] real = reshape([0.0, 0.5, 0.0,
                                              0.5, 0.5, 0.5,
                                              0.0, 0.5, 0.0], {1..3, 1..3});
var input: [1..n, 1..n] real = 1;


forall i in 1..n {
    forall j in 1..n {
        writeln(+ reduce (input[i - 1..i + 1, j - 1..j + 1] * constants));
    }
}
