set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# general target flags
unset(PROJECT_FLAGS)
unset(PROJECT_LINKER_FLAGS)
unset(PROJECT_INCLUDE_DIRS)

# standard compiler flags
set(PROJECT_FLAGS
	-march=native
	-mtune=native
	-fno-math-errno
	-Ofast
    -fopenmp
	)

# set warning flags for debug
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	if(CMAKE_C_COMPILER_ID STREQUAL "Clang" OR
		CMAKE_C_COMPILER_ID STREQUAL "GNU")
		set(PROJECT_FLAGS ${PROJECT_FLAGS}
			-Wall
			-Wextra)
		if(${WERROR})
			set(PROJECT_FLAGS ${PROJECT_FLAGS} -Werror)
		endif()
	else()
		message(WARNING "Warnings not supported for current compiler.")
	endif()
endif()

set(PROJECT_INCLUDE_DIRS ${PROJECT_INCLUDE_DIRS}
	"${PROJECT_SOURCE_DIR}/include")

if(${SANITISE})
	include(sanitise)
endif()

if(MPI_BACKEND STREQUAL "openmpi")
	set(MPI_EXECUTABLE_SUFFIX ".openmpi")
else()
	set(MPI_EXECUTABLE_SUFFIX ".mpich")
endif()
find_package(MPI REQUIRED COMPONENTS C)

if(NOT DEFINED ENV{GASNet_ROOT} AND NOT GASNet_ROOT_DIR)
	MESSAGE(FATAL_ERROR "Please set environment variable GASNet_ROOT "
		"to your gasnet installation path OR set the CMake "
		"GASNet_ROOT_DIR variable")
endif()

find_package(GASNet REQUIRED)

# create the common C target
add_library(cinterface INTERFACE)
target_compile_options(cinterface INTERFACE ${PROJECT_FLAGS})
target_include_directories(cinterface INTERFACE ${PROJECT_INCLUDE_DIRS})
target_link_options(cinterface INTERFACE ${PROJECT_LINKER_FLAGS} -fopenmp)

add_subdirectory(src)
