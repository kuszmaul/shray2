include(language_standard)

set(BLA_VENDOR OpenBLAS)
set(BLA_SIZEOF_INTEGER 8)
find_package(BLAS REQUIRED)

# util lib
add_subdirectory(util)

# shray examples
add_subdirectory(chapel)

# fortran examples
add_subdirectory(fortran)

# global arrays examples
add_subdirectory(globalarrays)

# mpi examples
add_subdirectory(mpi)

# shray examples
add_subdirectory(shray)

# upc examples
add_subdirectory(upc)

# gasnet examples
add_subdirectory(gasnet)

# omp reference example (cg)
add_subdirectory(omp)
