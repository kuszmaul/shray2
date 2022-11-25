/* Use ./2dstencil --n=20000 --iterations=5 to run with different arguments. */
config const n: int = 10000;
config const iterations: int = 10;

const constants = [[0.0, 0.5, 0.0],
                   [0.5, 0.5, 0.5],
                   [0.0, 0.5, 0.0]];

proc relax(input: [1..n, 1..n] real)
{
    var output: [1..n, 1..n] real;

    /* Inner part */
    forall (i, j) in [2..n - 1, 2..n - 1] do
        output[i, j] = sum(input[i - 1..i + 1, j - 1..j + 1] * constants);

    /* Boundary */
    output[1, 1..n] = input[1, 1..n];
    output[n, 1..n] = input[n, 1..n];
    output[1..n, 1] = input[1..n, 1];
    output[1..n, n] = input[1..n, n];

    return output;
}

proc stencil(input: [1..n, 1..n] real)
{
    var copy: [1..n, 1..n] real = input;

    for t in [1..iterations] do
        input = relax(input);

    return input;
}

var input: [1..n, 1..n] real = 1;
var result: real;

//writeln(sum(stencil(input, output)));
