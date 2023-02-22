#!/bin/sh

#SBATCH --account=csmpi
#SBATCH --partition=csmpi_short
#SBATCH --nodes=1
#SBATCH --output=build.out
#SBATCH --time=00:05:00

set -eu

builddir=build
cmake \
	-DEXAMPLES=ON \
	-DChapel_ROOT_DIR="${HOME}/.local" \
	-DGASNet_CONDUIT=udp \
	-DGASNet_ROOT_DIR="${HOME}/.local" \
	-DSANITISE=OFF \
    -DGFORTRAN_LIB_DIR="/usr/lib/gcc/x86_64-linux-gnu/11" \
    -DMPI_EXECUTABLE_SUFFIX=.mpich \
	-S . \
	-B "$builddir"
cmake --build "$builddir" -j
