# set the c/c++ language options for all targets defined hereafter
# define any subproject(s) prior to these statements to avoid conflict
include(language_standard)

# build project library
foreach(suffix
		""
		_debug
		_profile
		_graph)
	if(suffix STREQUAL "_debug")
		set(EXTRA_DEFINES DEBUG)
	elseif(suffix STREQUAL "_profile")
		set(EXTRA_DEFINES PROFILE)
	elseif(suffix STREQUAL "_graph")
		set(EXTRA_DEFINES GRAPH)
	endif()

	set(LIB_TARGET "${PROJECT_NAME}${suffix}")
	add_library("${LIB_TARGET}" STATIC
		shray.c)
	target_include_directories("${LIB_TARGET}"
		PUBLIC
			$<INSTALL_INTERFACE:include>
			$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>)
	target_link_libraries("${LIB_TARGET}"
		PUBLIC
			GASNet::GASNet
		PRIVATE
			cinterface)
	target_compile_definitions("${LIB_TARGET}"
		PRIVATE
			${EXTRA_DEFINES})
endforeach(suffix)
