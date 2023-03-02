find_program(CMAKE_Chapel_COMPILER
	NAMES chpl
	DOC "Chapel compiler."
	PATHS ${Chapel_ROOT_DIR})

mark_as_advanced(CMAKE_Chapel_COMPILER)

message(STATUS "Detected Chapel compiler: ${CMAKE_Chapel_COMPILER}")

set(CMAKE_Chapel_SOURCE_FILE_EXTENSIONS chpl)
set(CMAKE_Chapel_COMPILER_ENV_VAR "Chapel")

# Configure variables set in this file for fast reload later on
configure_file(${CMAKE_CURRENT_LIST_DIR}/CMakeChapelCompiler.cmake.in
               ${CMAKE_PLATFORM_INFO_DIR}/CMakeChapelCompiler.cmake)
