set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# general target flags
unset(PROJECT_FLAGS)
unset(PROJECT_LINKER_FLAGS)
unset(PROJECT_INCLUDE_DIRS)

# standard compiler flags
set(PROJECT_FLAGS
	-march=native
	-mtune=native
	-ffast-math
	-fno-math-errno
    -O3
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

include(gasnet)

# create the common C target
add_library(cinterface INTERFACE)
target_compile_options(cinterface INTERFACE ${PROJECT_FLAGS})
target_include_directories(cinterface INTERFACE ${PROJECT_INCLUDE_DIRS})
target_link_options(cinterface INTERFACE ${PROJECT_LINKER_FLAGS})

add_subdirectory(src)
