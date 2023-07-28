// This version ran in...
//   618.233 with default arguments, linear search for sparse domain this()
//    62.169 with --cflags=-O3, linear search
//    25.4556 with --cflags=-O3, binary search
prototype module CG {
use LayoutCS, CGMakeA, Time, IO, BlockDist, AllLocalesBarriers;

type elemType = real(64);

enum classVals {S, W, A, B, C, D};


const Class = {classVals.S..classVals.D};
const probSizes:   [Class] int = [ 1400, 7000, 14000, 75000, 150000, 150000 ],
      nonZeroes:   [Class] int = [ 7, 8, 11, 13, 15, 21 ],
      shifts:      [Class] int = [ 10, 12, 20, 60, 110, 500 ],
      numIters:    [Class] int = [ 15, 15, 15, 75, 75, 100 ],
      verifyZetas: [Class] elemType =[ 8.5971775078648, 10.362595087124,
                                        17.130235054029,  22.712745482631,
                                        28.973605592845,  52.5145321058 ];

config const probClass = classVals.S;

config const n = probSizes(probClass),
             nonzer = nonZeroes(probClass),
             shift = shifts(probClass),
             niter = numIters(probClass),
             zetaVerifyValue = verifyZetas(probClass);

config const numTrials = 1,
             verbose = true,
             debug = false,
             printTiming = true,
             printError = true;


proc main() {
  /* To reduce the chance of me messing up, we first build A redundantly on each node,
     and then copy the local parts over to a distributed Ad. This limits the size of A to
     the physical memory of one node, but that is enough for our experiments. */
  const DenseSpace = {1..n, 1..n};
  const MatrixSpace: sparse subdomain(DenseSpace) dmapped(new dmap(new CS()))
                   = genAIndsSorted(elemType, n, nonzer, shift);
  var A: [MatrixSpace] elemType;

    stderr.writeln("Start program");

  for (ind, v) in makea(elemType, n, nonzer, shift) {
    A(ind) += v;
  }

    stderr.writeln("A initialized");

  const DenseSpaceD: domain(2) dmapped Block(boundingBox=DenseSpace)
    = DenseSpace;
  var MatrixSpaceD: sparse subdomain(DenseSpaceD);// = MatrixSpace;

  /* Assignment is not supported, adding directly is O(n^2) to account
   * for duplicates? */
  var idxBuf = MatrixSpace.createIndexBuffer(size=2 * nonzer);
  for i in MatrixSpace {
      idxBuf.add(i);
  }
  idxBuf.commit();

    stderr.writeln("MatrixSpaceD initialized");

  var AD: [MatrixSpaceD] elemType;

    coforall loc in Locales do on loc {
        AD(AD.localSubdomain()) = A(AD.localSubdomain());
        //for i in AD.localSubdomain() {
        //    writeln("Locale ", here.id, " AD(", i, ") = ", AD(i));
        //    AD(i) = A(i);
        //}
        allLocalesBarrier.barrier();
    }


    stderr.writeln("AD initialized");

  const VectorSpace = {1..n};
//  var X: [VectorSpace] elemType,
//      zeta = 0.0;
  const VSD: domain(1) dmapped Block(boundingBox=VectorSpace) = VectorSpace;
  var X: [VSD] elemType,
        zeta = 0.0;

  for trial in 1..numTrials {
    X = 1.0;

    const startTime = timeSinceEpoch().totalSeconds();

    for it in 1..niter {
      const (Z, rnorm) = conjGrad(A, X);

      zeta = shift + 1.0 / + reduce (X*Z);

      if verbose then stderr.writeln(it, " ", rnorm, " ", zeta);

      X = (1.0 / sqrt(+ reduce(Z*Z))) * Z;
    }

    const runtime = timeSinceEpoch().totalSeconds() - startTime;

    var mflops: elemType = 1e-6 * 2 * niter * n * (3 + nonzer * (nonzer + 1) +
            25 * (5 + nonzer * (nonzer + 1) + 3)) / runtime;

    if printTiming then stderr.writeln("Execution time = ", runtime);
    if printTiming then stderr.writeln("Mflops/s = ", mflops);
    if printTiming then writeln(mflops);

    if (zetaVerifyValue != 0.0) {
      const epsilon = 1.0e-10;
      if (abs(zeta - zetaVerifyValue) <= epsilon) {
        stderr.writeln("Verification successful");
        stderr.writeln("Zeta is: ", zeta);
        if printError then stderr.writeln("Error is: ", zeta - zetaVerifyValue);
      } else {
        stderr.writeln("Verification failed");
        stderr.writeln("Zeta is: ", zeta);
        stderr.writeln("The correct zeta is: ", zetaVerifyValue);
      }
    } else {
      stderr.writeln("No verification performed");
    }
  }
}


proc conjGrad(A: [?MatDom], X: [?VectDom]) {
  const cgitmax = 25;

  var Z: [VectDom] elemType = 0.0,
      R = X,
      P = R;
  var rho = + reduce R**2;

  for cgit in 1..cgitmax {
    // WANT (a partial reduction):
    //    const Q = + reduce(dim=2) [(i,j) in MatDom] (A(i,j) * P(j));
    // INSTEAD OF:
    var Q: [VectDom] elemType;
//    [i in MatDom.dim(0)] Q(i) = + reduce [j in MatDom.dimIter(1,i)] (A(i,j) * P(j));
    //
    coforall loc in Locales do on loc {
        for i in VectDom.localSubdomain() {
            Q(i) = + reduce [j in MatDom.dimIter(1, i)] (A(i, j) * P(j));
        }
        allLocalesBarrier.barrier();
    }

    const alpha = rho / + reduce (P*Q);
    Z += alpha*P;
    R -= alpha*Q;

    const rho0 = rho;
    rho = + reduce R**2;
    const beta = rho / rho0;
    P = R + beta*P;
  }
  // WANT (a partial reduction):
  //      R = + reduce(dim=2) [(i,j) in MatDom] (A(i,j) * Z(j));
  // INSTEAD OF:
//  [i in MatDom.dim(0)] R(i) = + reduce [j in MatDom.dimIter(1,i)] (A(i,j) * Z(j));
  //

  coforall loc in Locales do on loc {
    for i in VectDom.localSubdomain() {
        R(i) = + reduce [j in MatDom.dimIter(1, i)] (A(i, j) * Z(j));
    }
    allLocalesBarrier.barrier();
  }
  const rnorm = sqrt(+ reduce ((X-R)**2));

  return (Z, rnorm);
}

}

module CGMakeA {

  use Random, Sort;

  config const rcond = 0.1;

  iter makea(type elemType, n, nonzer, shift) {
    var v: [1..nonzer+1] elemType, // BLC: insert domains? or grow as necessary?
        iv: [1..nonzer+1] int;

    var size = 1.0;
    const ratio = rcond ** (1.0 / n);

    var randStr = new NPBRandomStream(eltType=real, seed=314159265);
    randStr.getNext();   // drop a value on floor to match NPB version

    for iouter in 1..n {
      var nzv = nonzer;

      sprnvc(elemType, n, nzv, v, iv, randStr);
      vecset(v, iv, nzv, iouter, 0.50);

      // BLC: replace with zippered loop over iv or iv(1..nzv)?
      for ivelt in 1..nzv {
        const jcol = iv(ivelt),
              scale = size * v(ivelt);

        // BLC: replace with zippered loop over iv or iv(1..nzv)?
        for ivelt1 in 1..nzv {
          const irow = iv(ivelt1);

          yield ((irow, jcol), v(ivelt1)*scale);
        }
      }
      size *= ratio;
    }

    for i in 1..n {
      yield ((i, i), rcond - shift);
    }

  }


  iter genAInds(type elemType, n, nonzer, shift) {
    for (ind, val) in makea(elemType, n, nonzer, shift) {
      yield ind;
    }
  }


  iter genAIndsSorted(type elemType, n, nonzer, shift) {
    // build associative domain of indices
    var Inds: domain(index(2));
    for i in genAInds(elemType, n, nonzer, shift) {
      Inds += i;
    }
    //  writeln("Inds is: ", Inds);

    // copy into arithmetic domain/array
    var IndArr: [1..Inds.size] index(2);
    for (i,j) in zip(Inds, 1..) {
      IndArr[j] = i;
    }
    //  writeln("IndArr is: ", IndArr);

    // sort indices
    sort(IndArr);

    //  writeln("After sort, IndArr is: ", IndArr);

    for i in IndArr {
      yield i;
    }

    // TODO: should "free" local domains/arrays here by making degenerate
  }


  proc sprnvc(type elemType, n, nz, v, iv, randStr) {
    var nn1 = 1;
    while (nn1 < n) do nn1 *= 2;

    var indices: domain(int);

    for nzv in 1..nz {
      var vecelt: elemType,
          ind: int;

      do {
        vecelt = randStr.getNext();
        ind = (randStr.getNext() * nn1):int + 1;
      } while (ind > n || indices.contains(ind));

      v(nzv) = vecelt;
      iv(nzv) = ind;
      indices += ind;
    }
  }


  proc vecset(v, iv, inout nzv, i, val) {
    var set = false;
    for k in 1..nzv {
      if (iv(k) == i) {
        v(k) = val;
        set = true;
      }
    }
    if (!set) {
      nzv += 1;
      v(nzv) = val;
      iv(nzv) = i;
    }
  }
}
