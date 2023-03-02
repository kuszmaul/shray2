if(NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
	message(FATAL_ERROR
		"Sanitise is only supported for CMAKE_BUILD_TYPE=DEBUG.")
endif()

if(${WERROR})
	set(PROJECT_FLAGS ${PROJECT_FLAGS} -fno-sanitize-recover=all)
endif()

if(CMAKE_C_COMPILER_ID STREQUAL "Clang")
	set(EXTRA_SAN_FLAGS "-fsanitize=integer" "-fsanitize=nullability")

	if(CMAKE_C_COMPILER_VERSION VERSION_GREATER_EQUAL 7)
		set(EXTRA_SAN_FLAGS ${EXTRA_SAN_FLAGS}
			"-fsanitize=implicit-conversion")
	elseif(NOT SANITISE_MSG_SHOWN)
		message(STATUS "Some sanitise options require Clang >= 7")
	endif()
else()
	unset(EXTRA_SAN_FLAGS)

	if(NOT SANITISE_MSG_SHOWN)
		message(STATUS "Some sanitise options not supported for \
current compiler")
	endif()
endif()

foreach(flag -fsanitize=undefined -fsanitize=address ${EXTRA_SAN_FLAGS})
	set(PROJECT_FLAGS ${PROJECT_FLAGS} ${flag})
	set(PROJECT_LINKER_FLAGS ${PROJECT_LINKER_FLAGS} ${flag})
endforeach(flag)

if(NOT SANITISE_MSG_SHOWN)
	set(SANITISE_MSG_SHOWN ON CACHE BOOL "Shown sanitise options message.")
	mark_as_advanced(SANITISE_MSG_SHOWN)
endif()
