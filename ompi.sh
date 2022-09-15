#!/bin/bash

sed -i 's/mpicc.mpich/mpicc.openmpi/g' Makefile
sed -i 's/mpirun.mpich/mpirun.openmpi/g' Makefile

make clean
make
