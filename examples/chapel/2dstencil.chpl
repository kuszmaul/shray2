use Time;
use IO;

/* Use ./2dstencil --n=20000 --iterations=5 to run with different arguments. */
config const n: int = 10000;
config const iterations: int = 10;

const constants: [1..3, 1..3] real = reshape([0.0, 0.5, 0.0,
                                              0.5, 0.5, 0.5,
                                              0.0, 0.5, 0.0], {1..3, 1..3});

proc relax(input: [1..n, 1..n] real)
{
    var output: [1..n, 1..n] real;

    /* Inner part */
//    forall (i, j) in {2..n - 1, 2..n - 1} do
//        output[i, j] = + reduce (input[i - 1..i + 1, j - 1..j + 1] * constants);
    forall (i, j) in {2..n - 1, 2..n - 1} do
        for di in -1..1 do
            for dj in -1..1 do
                output[i, j] += input[i + di, j + dj] * constants[2 + di, 2 + dj];

    /* Boundary */
    output[1, ..] = input[1, ..];
    output[n, ..] = input[n, ..];
    output[.., 1] = input[.., 1];
    output[.., n] = input[.., n];

    return output;
}

proc stencil(input: [1..n, 1..n] real)
{
    for t in 1..iterations do
        input = relax(input);

    return input;
}

var input: [1..n, 1..n] real = 1;
var watch: Timer;
watch.start();

input = stencil(input);
watch.stop();

stdout.writeln(9.0 * (n - 2) * (n - 2) * iterations / watch.elapsed() / 1000000000.0, '\n');
