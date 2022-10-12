#!/bin/bash

# Matrix multiplication strong scaling on n = 4000.
P=1
PMAX=4
CACHESIZE=384000000
export SHRAY_CACHELINE=1

rm blas.out

while [ $P -le $PMAX ]
do
	export SHRAY_CACHESIZE=$CACHESIZE
	mpirun -n $P ../bin/blas 4000 >> blas.out
    let P=2*$P
    let CACHESIZE=$CACHESIZE/2
done
