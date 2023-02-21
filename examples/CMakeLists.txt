include(language_standard)

find_package(BLAS REQUIRED)

# util lib
add_subdirectory(util)

# shray examples
#add_subdirectory(chapel)

# fortran examples
add_subdirectory(fortran)

# global arrays examples
#add_subdirectory(globalarrays)

# mpi examples
add_subdirectory(mpi)

# shray examples
add_subdirectory(shray)

# upc examples
#add_subdirectory(upc)
