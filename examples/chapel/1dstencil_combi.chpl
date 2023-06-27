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
//  var dom: domain(1) = input.localSubdomain();
//  var start: int = dom.first / BLOCK;
//  var end: int = dom.last / BLOCK;
  var start: int = (n / BLOCK + numLocales - 1) / numLocales * here.id;
  var end: int = (n / BLOCK + numLocales - 1) / numLocales * (here.id + 1);


  /* Multithreaded execution, default uses one thread per core
   * We copy in a loop instead of array slices to mark local accesses */
  forall row in start..end - 1 {
    var inBuffer: [0..BLOCK + 2 * iterations - 1] real(32);
    var outBuffer: [0..BLOCK + 2 * iterations - 1] real(32);

    if (row == 0) {
      for i in 0 .. BLOCK + iterations - 1 {
        inBuffer(i) = input.localAccess(i);
      }
      left(BLOCK, iterations, inBuffer(0..BLOCK + iterations - 1),
                              outBuffer(0..BLOCK + iterations - 1));
      for i in 0 .. BLOCK - 1 {
        output.localAccess(i) = outBuffer(i);
      }
    } else if (row == n / BLOCK - 1) {
      for i in 0 .. BLOCK + iterations - 1 {
        inBuffer(i) = input.localAccess(row * BLOCK - iterations + i);
      }
      right(BLOCK, iterations, inBuffer(0..BLOCK + iterations - 1),
                               outBuffer(0..BLOCK + iterations - 1));
      for i in 0 .. BLOCK - 1 {
        output.localAccess(row * BLOCK + i) = outBuffer(iterations + i);
      }
    } else {
      /* The first and last block will access some elements from neighbouring
       * locales. */
      if (row != start && row != end - 1) {
        for i in 0 .. BLOCK + 2 * iterations - 1 {
          inBuffer(i) = input.localAccess(row * BLOCK - iterations + i);
        }
      } else {
        for i in 0 .. BLOCK + 2 * iterations - 1 {
          inBuffer(i) = input(row * BLOCK - iterations + i);
        }
      }
      middle(BLOCK, iterations, inBuffer, outBuffer);
      for i in 0 .. BLOCK - 1 {
        output.localAccess(row * BLOCK + i) = outBuffer(iterations + i);
      }
    }
  }
}

proc Stencil(n: int, input: [0..n - 1] real(32), output: [0..n - 1] real(32), iterations: int)
{
  for t in 1..iterations / TIMEBLOCK {
    StencilBlocked(n, input, output, TIMEBLOCK);
    input <=> output;
    allLocalesBarrier.barrier();
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
  // Execute on each node (locale)
  coforall loc in Locales do on loc {
    startCommDiagnosticsHere();
    Stencil(N, input, output, ITERATIONS);
    stopCommDiagnosticsHere();
  }
  watch.stop();

  printCommDiagnosticsTable();
  if (here.id == 0) then
    stdout.writeln(5.0 * (N - 2) * ITERATIONS / watch.elapsed() / 1000000000, '\n');
}
