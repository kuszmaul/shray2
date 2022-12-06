#######################################################################
# Specify system specific libraries for building the test programs
#######################################################################

GASNET_CONDUIT = /usr/local/gasnet/include/mpi-conduit/mpi-seq.mak

# Compilers
CC = gcc
FORTRAN_C = gfortran
MPICC = mpicc
CHAPEL_C = chpl
UPCC = upcc

# Linker flags for libraries
COARRAY_FORTRAN = -lcaf_openmpi
BLAS = -lblis64
GLOBALARRAYS = -lga
LAPACK = -llapack
SCALAPACK = -lscalapack-openmpi
FORTRANLIB = -lgfortran
ARMCI = -larmci
