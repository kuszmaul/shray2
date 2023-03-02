# This module produces the "GlobalArrays" link target which carries with it all
# the necessary interface properties. Very simple and naive implementation.
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
		${LIBARMCI_PATH}
		MPI::MPI_C
		m
		)
