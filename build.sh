#!/bin/sh

#SBATCH --account=csmpi
#SBATCH --partition=csmpi_long
#SBATCH --nodes=1
#SBATCH --cpus-per-task=8
#SBATCH --mem=16GB
#SBATCH --time=00:15:00
#SBATCH --output=build.out

set -eu

# Required to force Chapel applications to use gasnet udp-conduit.
export CHPL_COMM=gasnet
export CHPL_COMM_SUBSTRATE=udp
export CHPL_TARGET_CPU=native

builddir=build
[ -d "$builddir" ] && rm -r "$builddir"

cmake \
	-DEXAMPLES=ON \
	-DChapel_ROOT_DIR="${HOME}/.local" \
	-DGASNet_CONDUIT=mpi \
	-DGASNet_THREADING=par \
	-DGASNet_ROOT_DIR="${HOME}/.local" \
	-DSANITISE=OFF \
	-S . \
	-B "$builddir"
cmake --build "$builddir" -j
