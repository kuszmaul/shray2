# 3.13 required for target_link_options
cmake_minimum_required(VERSION 3.13 FATAL_ERROR)
list(INSERT CMAKE_MODULE_PATH 0 "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# set build type to debug if none is specified
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
	set(CMAKE_BUILD_TYPE Debug CACHE STRING
		"Choose the type of build." FORCE)
	set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
		"Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

# options
option(EXAMPLES "Build the examples." OFF)
option(SRC "Build primary libraries and executables." ON)
option(SANITISE "Add -fsanitize=undefined flags." OFF)
option(WERROR "Make all warnings into errors." OFF)
option(TEST "Build tests." OFF)

# disable base languages
unset(PROJECT_LANGUAGES)

if(${SRC})
	set(PROJECT_LANGUAGES ${PROJECT_LANGUAGES} C Fortran)
endif()

project(shray2
	VERSION 0.1.0
	DESCRIPTION "API for DSM"
	LANGUAGES ${PROJECT_LANGUAGES})
set(PROJECT_VERSION_SUFFIX "") # alpha/beta/rc, e.g. "-rc0"
set(PROJECT_VERSION "${PROJECT_VERSION}${PROJECT_VERSION_SUFFIX}")
set(PROJECT_AUTHOR "Thomas Koopman")
set(PROJECT_COPYRIGHT "2022, Thomas Koopman")
set(PROJECT_MAIL "thomas.koopman@ru.nl")
# only set CMAKE variant when local name matches CMAKE name
# this avoids clashing when being used as a subproject
if(PROJECT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
	set(CMAKE_PROJECT_VERSION "${PROJECT_VERSION}")
	set(CMAKE_PROJECT_VERSION_SUFFIX "${PROJECT_VERSION_SUFFIX}")
	set(CMAKE_PROJECT_AUTHOR "${PROJECT_AUTHOR}")
	set(CMAKE_PROJECT_COPYRIGHT "${PROJECT_COPYRIGHT}")
	set(CMAKE_PROJECT_MAIL "${PROJECT_MAIL}")
endif()

# base options for installation directories
include(GNUInstallDirs)

if(${SRC})
	include(src)
elseif(${EXAMPLES} OR ${SANITISE} OR ${TEST})
	message(FATAL_ERROR "At least one flag enabled that requires SRC=ON.")
endif()

if(${EXAMPLES})
	add_subdirectory(examples)
endif()

if(${TEST})
	enable_testing()
	add_subdirectory(tests)
endif()
