#!/bin/sh

set -eu

builddir=build
cmake \
	-DEXAMPLES=ON \
	-DGASNet_CONDUIT=mpi \
	-DGASNet_ROOT_DIR="/usr/local/gasnet" \
	-DSANITISE=ON \
	-DOSHCC=/home/thomas/repos/shmemBuild/bin/oshcc \
	-S . \
	-B "$builddir"
cmake --build "$builddir" -j

# -DGASNet_ROOT_DIR="/home/ximin/tmp/gasnet/install_segmenteverything_nophsm" \
