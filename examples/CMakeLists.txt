include(language_standard)

find_package(BLAS REQUIRED)

# util lib
add_subdirectory(util)

# shray examples
add_subdirectory(chapel)

# fortran examples
add_subdirectory(fortran)

# global arrays examples
add_subdirectory(globalarrays)

# shray examples
add_subdirectory(shray)
