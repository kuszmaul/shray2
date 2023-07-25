#!/bin/sh

#SBATCH --account=csmpi
#SBATCH --partition=csmpi_fpga_short
#SBATCH --nodes=1
#SBATCH --cpus-per-task=8
#SBATCH --mem=16GB
#SBATCH --time=00:05:00
#SBATCH --output=build.out

set -eu

# Required to force Chapel applications to use gasnet udp-conduit.
export CHPL_COMM=gasnet
export CHPL_COMM_SUBSTRATE=udp
export CHPL_TARGET_CPU=native

builddir=build_openmpi
[ -d "$builddir" ] && rm -r "$builddir"

cmake \
	-DEXAMPLES=ON \
	-DChapel_ROOT_DIR="$HOME/download/chapel/chapel-1.31.0/bin/linux64-x86_64" \
	-DGASNet_CONDUIT=mpi \
	-DGASNet_THREADING=parsync \
	-DGASNet_ROOT_DIR="$HOME/.local" \
	-DUPC_ROOT_DIR="$HOME/.local/bin" \
	-DMPI_BACKEND=openmpi \
	-DGFORTRAN_LIB_DIR="/usr/lib/gcc/x86_64-linux-gnu/11" \
	-DSANITISE=OFF \
	-S . \
	-B "$builddir"
cmake --build "$builddir" -j

builddir=build_mpich
[ -d "$builddir" ] && rm -r "$builddir"

cmake \
	-DEXAMPLES=ON \
	-DChapel_ROOT_DIR="$HOME/download/chapel/chapel-1.31.0/bin/linux64-x86_64" \
	-DGASNet_CONDUIT=mpi \
	-DGASNet_THREADING=parsync \
	-DGASNet_ROOT_DIR="$HOME/opt/mpich" \
	-DUPC_ROOT_DIR="$HOME/.local/bin" \
	-DARMCI_LIB_DIR="$HOME/opt/mpich/lib" \
	-DMPI_BACKEND=mpich \
	-DGFORTRAN_LIB_DIR="/usr/lib/gcc/x86_64-linux-gnu/11" \
	-DSANITISE=OFF \
	-S . \
	-B "$builddir"
cmake --build "$builddir" -j
