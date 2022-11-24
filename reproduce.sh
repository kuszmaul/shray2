#!/bin/bash

export SHRAY_CACHELINE=1
export SHRAY_CACHESIZE=200000

mpirun -n 4 bin/nbody_debug 2000 2 &> nbody.out
grep "\[node 0" nbody.out > nbody0.out
grep "\[node 1" nbody.out > nbody1.out
grep "\[node 2" nbody.out > nbody2.out
grep "\[node 3" nbody.out > nbody3.out
