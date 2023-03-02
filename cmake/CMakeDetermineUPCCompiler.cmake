find_program(CMAKE_UPC_COMPILER
	NAMES upcc
	DOC "UPC compiler."
	PATHS ${UPC_ROOT_DIR})

mark_as_advanced(CMAKE_UPC_COMPILER)

message(STATUS "Detected UPC compiler: ${CMAKE_UPC_COMPILER}")

set(CMAKE_UPC_SOURCE_FILE_EXTENSIONS upc)
set(CMAKE_UPC_OUTPUT_EXTENSION .o)
set(CMAKE_UPC_COMPILER_ENV_VAR "UPC")

# Configure variables set in this file for fast reload later on
configure_file(${CMAKE_CURRENT_LIST_DIR}/CMakeUPCCompiler.cmake.in
               ${CMAKE_PLATFORM_INFO_DIR}/CMakeUPCCompiler.cmake)
