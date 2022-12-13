# This module produces the "GlobalArrays" link target which carries with it all
# the necessary interface properties. Very simple and naive implementation, it
# does not account for all of the various combinations of e.g. SCALAPACK.
# Assumes MPI::MPI_C interface is available.
#
# The following variables can be set to influence search properties.
#
# GlobalArrays_INCLUDE_DIR  - Directory to use when searching for GA headers. If
#                             specified then this search path will be used.
# GlobalArrays_LIB_DIR      - Directory to use when searching for GA libs. If
#                             specified then this search path will be used.
# ARMCI_LIB_DIR             - Directory to use when searching for ARMCI libs. If
#                             specified then this search path will be used.
# GFORTRAN_LIB_DIR          - Directory to use when searching for GFortran libs.
#                             if specified then this search path will be used.
# SCALAPCK_LIB_DIR          - Directory to use when searching for SCALAPCK libs.
#                             if specified then this search path will be used.
# SCALAPCK_TYPE             - Directory to use when searching for SCALAPCK libs.
#                             if specified then this search path will be used.

find_package(LAPACK REQUIRED)
find_package(BLAS REQUIRED)

if(NOT GlobalArrays_FOUND)
	find_library(LIBGA_PATH
		NAMES ga
		PATHS ${GlobalArrays_LIB_DIR}
		DOC "Path to libga")
	if(NOT LIBGA_PATH)
		message(FATAL_ERROR "Could not find global arrays library, please set GlobalArrays_LIB_DIR")
	endif()

	find_file(INCLUDE_GA_PATH
		NAMES ga.h
		PATHS ${GlobalArrays_INCLUDE_DIR}
		DOC "Path to global arrays header directory")
	if(NOT INCLUDE_GA_PATH)
		message(FATAL_ERROR "Could not find global arrays headers, please set GlobalArrays_INCLUDE_DIR")
	endif()
	get_filename_component(GA_INCLUDE_DIR ${INCLUDE_GA_PATH} DIRECTORY)
	message(STATUS "Found GA: ${GA_INCLUDE_DIR};${LIBGA_PATH}")

	find_library(LIBARMCI_PATH
		NAMES armci
		PATHS ${ARMCI_LIB_DIR}
		DOC "Path to libarmci")
	if(NOT LIBARMCI_PATH)
		message(FATAL_ERROR "Could not find armci library, please set ARMCI_LIB_DIR")
	endif()
	message(STATUS "Found ARMCI: ${LIBARMCI_PATH}")

	find_library(LIBGFORTRAN_PATH
		NAMES gfortran
		PATHS ${GFORTRAN_LIB_DIR}
		DOC "Path to libgfortran")
	if(NOT LIBGFORTRAN_PATH)
		message(FATAL_ERROR "Could not find gfortran library, please set GFORTRAN_LIB_DIR")
	endif()
	message(STATUS "Found gfortran: ${LIBGFORTRAN_PATH}")

	find_library(LIBSCALAPACK_PATH
		NAMES scalapack-openmpi
		PATHS ${SCALAPACK_LIB_DIR}
		DOC "Path to scalapack")
	if(NOT LIBSCALAPACK_PATH)
		message(FATAL_ERROR "Could not find scalapack library, please set SCALAPACK_LIB_DIR")
	endif()
	message(STATUS "Found scalapack: ${LIBSCALAPACK_PATH}")

	set(GlobalArrays_FOUND ON CACHE BOOL "Found Global Arrays.")
	mark_as_advanced(GlobalArrays_FOUND)
endif()

add_library(GlobalArrays::GlobalArrays INTERFACE IMPORTED)
target_include_directories(GlobalArrays::GlobalArrays
	INTERFACE
		${GA_INCLUDE_DIR})
target_link_libraries(GlobalArrays::GlobalArrays
	INTERFACE
		${LIBGA_PATH}
		BLAS::BLAS
		MPI::MPI_C
		LAPACK::LAPACK
		m
		${LIBARMCI_PATH}
		${LIBGFORTRAN_PATH}
		${LIBSCALAPACK_PATH})
