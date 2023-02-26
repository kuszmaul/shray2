#!/bin/sh

#SBATCH --account=csmpi
#SBATCH --partition=csmpi_long
#SBATCH --nodes=1
#SBATCH --time=00:15:00
#SBATCH --output=build.txt

set -eu

# Required to force Chapel applications to use gasnet udp-conduit.
export CHPL_COMM=gasnet
export CHPL_COMM_SUBSTRATE=mpi

rm -rf build

builddir=build
cmake \
	-DEXAMPLES=ON \
	-DChapel_ROOT_DIR="${HOME}/.local" \
	-DGASNet_CONDUIT=mpi \
	-DGASNet_ROOT_DIR="/usr/local/gasnet" \
	-DSANITISE=OFF \
    -DGFORTRAN_LIB_DIR="/usr/lib/gcc/x86_64-linux-gnu/12" \
	-S . \
	-B "$builddir"
cmake --build "$builddir" -j
