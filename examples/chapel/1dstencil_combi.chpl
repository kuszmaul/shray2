use IO;
use BlockDist;
use Time;

const BLOCK: int = 10000;
const TIMEBLOCK: int = 100;
const a: real(32) = 0.5;
const b: real(32) = 0.33;
const c: real(32) = 0.25;

config const N = 1000000;
config const ITERATIONS = 500;

proc left(n: int, iterations: int, input: [0..n + iterations - 1] real(32),
    output: [0..n + iterations - 1] real(32))
{
  var temp: [0..n + iterations - 1] real(32);

  output[0] = input[0];

  for t in 1..iterations {
    for i in 1..n + iterations - t {
      output[i] = a * input[i - 1] +
                  b * input[i] +
                  c * input[i + 1];
    }
    if (t != iterations) {
      temp = input;
      input = output;
      output = input;
    }
  }
}

proc middle(n: int, iterations: int, input: [0..n + 2 * iterations - 1] real(32),
    output: [0..n + 2 * iterations - 1] real(32))
{
  var temp: [0..n + iterations - 1] real(32);

  for t in 1..iterations {
    for i in t..n + 2 * iterations - t {
      output[i] = a * input[i - 1] +
                  b * input[i] +
                  c * input[i + 1];
    }
    if (t != iterations) {
      temp = input;
      input = output;
      output = input;
    }
  }
}

proc right(n: int, iterations: int, input: [0..n + iterations - 1] real(32),
    output: [0..n + iterations - 1] real(32))
{
  var temp: [0..n + iterations - 1] real(32);

  output[n + iterations - 1] = input[n + iterations - 1];

  for t in 1..iterations {
    for i in t..n + iterations - 1 {
      output[i] = a * input[i - 1] +
                  b * input[i] +
                  c * input[i + 1];
    }
    if (t != iterations) {
      temp = input;
      input = output;
      output = input;
    }
  }
}

proc StencilBlocked(n: int, input: [0..n - 1] real(32), output: [0..n - 1] real(32), iterations: int)
{
  var inBuffer: [0..BLOCK + 2 * iterations - 1] real(32);
  var outBuffer: [0..BLOCK + 2 * iterations - 1] real(32);

  var start: int = (n / BLOCK + numLocales - 1) / numLocales * here.id;
  var end: int = min((n / BLOCK + numLocales - 1) / numLocales * (here.id + 1), n / BLOCK);

  coforall row in start..end - 1 {
    if (row == 0) {
      inBuffer(0 .. BLOCK + iterations - 1) = input(0 .. BLOCK + iterations - 1);
      left(BLOCK, iterations, inBuffer, outBuffer);
      output(0..BLOCK - 1) = outBuffer(0..BLOCK - 1);
    } else if (row == n / BLOCK - 1) {
      inBuffer(0 .. BLOCK + iterations - 1) =
         input(row * BLOCK - iterations .. (row + 1) * BLOCK - 1);
      right(BLOCK, iterations, inBuffer, outBuffer);
      output(row * BLOCK .. (row + 1) * BLOCK - 1) = outBuffer(iterations .. iterations + BLOCK - 1);
    } else {
      inBuffer(0 .. BLOCK + 2 * iterations - 1) =
          input(row * BLOCK - iterations .. (row + 1) * BLOCK + iterations - 1);
      right(BLOCK, iterations, inBuffer, outBuffer);
      output(row * BLOCK .. (row + 1) * BLOCK - 1) = outBuffer(iterations .. iterations + BLOCK - 1);
    }
  }
}

proc Stencil(n: int, input: [0..n - 1] real(32), output: [0..n - 1] real(32), iterations: int)
{
//  writeln("Hello from locale ", here.id);

  sync for t in 1..iterations / TIMEBLOCK {
    StencilBlocked(n, input, output, TIMEBLOCK);
    input <=> output;
  }
  if (iterations % TIMEBLOCK != 0) {
    StencilBlocked(n, input, output, iterations % TIMEBLOCK);
  } else {
    /* We did one buffer swap too many */
    input <=> output;
  }
}

proc main()
{
  const Space = {0..N};
  const BlockSpace = Space dmapped Block(boundingBox=Space);
  var input: [BlockSpace] real(32) = 1;
  var output: [BlockSpace] real(32) = 1;

  var watch: Timer;
  watch.start();
  coforall loc in Locales do on loc {
    Stencil(N, input, output, ITERATIONS);
  }
  watch.stop();

  if (here.id == 0) then
    stdout.writeln(5.0 * (N - 2) * ITERATIONS / watch.elapsed() / 1000000000, '\n');
}
