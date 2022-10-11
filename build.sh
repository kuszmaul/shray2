#!/bin/sh

set -eu

builddir=build
cmake \
	-DEXAMPLES=ON \
	-DGASNet_CONDUIT=mpi \
	-DGASNet_ROOT_DIR="/usr/local/gasnet" \
	-DSANITISE=ON \
	-S . \
	-B "$builddir"
cmake --build "$builddir"

# -DGASNet_ROOT_DIR="/home/ximin/tmp/gasnet/install_segmenteverything_nophsm" \
