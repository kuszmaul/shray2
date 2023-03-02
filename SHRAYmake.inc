#######################################################################
# Specify system specific libraries for building the test programs
#######################################################################

GASNET_CONDUIT = /home/tkoopman/.local/include/mpi-conduit/mpi-seq.mak

# Compilers
CC = gcc
FORTRAN_C = gfortran
MPICC = mpicc
CHAPEL_C = chpl
UPCC = upcc

# Linker flags for libraries
COARRAY_FORTRAN = -lcaf_openmpi
BLAS = -lcblas
GLOBALARRAYS =
LAPACK =
SCALAPACK =
FORTRANLIB =
ARMCI =
