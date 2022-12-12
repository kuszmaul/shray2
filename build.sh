#!/bin/sh

set -eu

builddir=build
cmake \
	-DEXAMPLES=ON \
	-DChapel_ROOT_DIR="/usr/local/bin" \
	-DGASNet_CONDUIT=mpi \
	-DGASNet_ROOT_DIR="/usr/local/gasnet" \
	-DSANITISE=ON \
	-S . \
	-B "$builddir"
cmake --build "$builddir" -j
