use IO;
use BlockDist;
use Time;
use AllLocalesBarriers;
use CommDiagnostics;

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
  output[0] = input[0];

  for t in 1..iterations {
    for i in 1..n + iterations - t - 1 {
      output[i] = a * input[i - 1] +
                  b * input[i] +
                  c * input[i + 1];
    }
    if (t != iterations) {
      input <=> output;
    }
  }
}

proc middle(n: int, iterations: int, input: [0..n + 2 * iterations - 1] real(32),
    output: [0..n + 2 * iterations - 1] real(32))
{
  for t in 1..iterations {
    for i in t..n + 2 * iterations - t - 1 {
      output[i] = a * input[i - 1] +
                  b * input[i] +
                  c * input[i + 1];
    }
    if (t != iterations) {
      input <=> output;
    }
  }
}

proc right(n: int, iterations: int, input: [0..n + iterations - 1] real(32),
    output: [0..n + iterations - 1] real(32))
{
  output[n + iterations - 1] = input[n + iterations - 1];

  for t in 1..iterations {
    for i in t..n + iterations - 2 {
      output[i] = a * input[i - 1] +
                  b * input[i] +
                  c * input[i + 1];
    }
    if (t != iterations) {
      input <=> output;
    }
  }
}

proc StencilBlocked(n: int, input: [0..n - 1] real(32), output: [0..n - 1] real(32), iterations: int)
{
  /* Cannot use localSubdomain because we divide the computation by a
   * block distribution on slices of size BLOCK, instead of single elements.*/
  var start: int = (n / BLOCK + numLocales - 1) / numLocales * here.id;
  var end: int = (n / BLOCK + numLocales - 1) / numLocales * (here.id + 1);

  /* Multithreaded execution, default uses one thread per core
   * We copy in a loop instead of array slices to mark local accesses */
  var bufDom = {0..BLOCK + 2 * iterations - 1};
  var inBuffer: [bufDom] real(32);
  var outBuffer: [bufDom] real(32);
  for row in start..end - 1 {
    if (row == 0) {
      inBuffer(0..BLOCK + iterations - 1) =
        input.localSlice(0..BLOCK + iterations - 1);
      left(BLOCK, iterations, inBuffer(0..BLOCK + iterations - 1),
           outBuffer(0..BLOCK + iterations - 1));
      output.localSlice(0..BLOCK - 1) = outBuffer(0..BLOCK - 1);
    } else if (row == n / BLOCK - 1) {
      inBuffer(0..BLOCK + iterations - 1) =
        input.localSlice(row * BLOCK - iterations..(row + 1) * BLOCK - 1);
      right(BLOCK, iterations, inBuffer(0..BLOCK + iterations - 1),
                               outBuffer(0..BLOCK + iterations - 1));
      output.localSlice(row * BLOCK..(row + 1) * BLOCK - 1) =
        outBuffer(iterations..BLOCK + iterations - 1);
    } else {
      /* The first and last block will access some elements from neighbouring
       * locales. */
      if (row != start && row != end - 1) {
        inBuffer(0..BLOCK + 2 * iterations - 1) =
          input.localSlice(row * BLOCK - iterations..(row + 1) * BLOCK + iterations - 1);
      } else {
        inBuffer(0..BLOCK + 2 * iterations - 1) =
          input(row * BLOCK - iterations..(row + 1) * BLOCK + iterations - 1);
      }
      middle(BLOCK, iterations, inBuffer, outBuffer);
      output.localSlice(row * BLOCK..(row + 1) * BLOCK - 1) =
        outBuffer(iterations..BLOCK + iterations - 1);
    }
  }
}

proc Stencil(n: int, input: [0..n - 1] real(32), output: [0..n - 1] real(32), iterations: int)
{
  for t in 1..iterations / TIMEBLOCK {
    StencilBlocked(n, input, output, TIMEBLOCK);
    allLocalesBarrier.barrier();
    input <=> output;
  }
  if (iterations % TIMEBLOCK != 0) {
    StencilBlocked(n, input, output, iterations % TIMEBLOCK);
  } else {
    /* We did one buffer swap too many */
    input <=> output;
  }
  allLocalesBarrier.barrier();
}

proc main()
{
  const Space = {0..N - 1};
  const BlockSpace = Space dmapped Block(boundingBox=Space);
  var input: [BlockSpace] real(32) = 1;
  var output: [BlockSpace] real(32) = 1;

  var watch: stopwatch;
  watch.start();
  coforall loc in Locales do on loc {
    Stencil(N, input, output, ITERATIONS);
  }
  watch.stop();

  printCommDiagnosticsTable();
  if (here.id == 0) then
    stdout.writeln(5.0 * (N - 2) * ITERATIONS / watch.elapsed() / 1000000000, '\n');
}
