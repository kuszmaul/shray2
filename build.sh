#!/bin/sh

set -eu

builddir=build
cmake \
	-DEXAMPLES=ON \
	-DChapel_ROOT_DIR="/usr/local/bin" \
	-DGASNet_CONDUIT=mpi \
	-DGASNet_ROOT_DIR="/usr/local/gasnet" \
	-DSANITISE=OFF \
    -DGFORTRAN_LIB_DIR="/usr/lib/gcc/x86_64-linux-gnu/12" \
	-S . \
	-B "$builddir"
cmake --build "$builddir" -j
