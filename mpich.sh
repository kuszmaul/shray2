#!/bin/bash

sed -i 's/mpicc.openmpi/mpicc.mpich/g' Makefile
sed -i 's/mpirun.openmpi/mpirun.mpich/g' Makefile

make clean
make
