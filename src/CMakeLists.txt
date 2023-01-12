include(language_standard)

SET(SHRAY_SUFFIX "_normal")
configure_file(shray.c.in shray_normal.c)
SET(SHRAY_SUFFIX "_debug")
configure_file(shray.c.in shray_debug.c)
SET(SHRAY_SUFFIX "_profile")
configure_file(shray.c.in shray_profile.c)

# build project library

set_source_files_properties(${CMAKE_CURRENT_BINARY_DIR}/shray_debug.c
    PROPERTIES COMPILE_DEFINITIONS DEBUG)
set_source_files_properties(${CMAKE_CURRENT_BINARY_DIR}/shray_profile.c
    PROPERTIES COMPILE_DEFINITIONS PROFILE)

set(LIB_TARGET "${PROJECT_NAME}")
add_library("${LIB_TARGET}" SHARED
	bitmap.c
	queue.c
	ringbuffer.c
	shray.c
    ${CMAKE_CURRENT_BINARY_DIR}/shray_debug.c
    ${CMAKE_CURRENT_BINARY_DIR}/shray_normal.c
    ${CMAKE_CURRENT_BINARY_DIR}/shray_profile.c)
target_compile_options("${LIB_TARGET}"
	PRIVATE
		-fPIC)
target_include_directories("${LIB_TARGET}"
	PUBLIC
        ${PROJECT_SOURCE_DIR}/src
		$<INSTALL_INTERFACE:include>
		$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>)
target_link_libraries("${LIB_TARGET}"
	PUBLIC
		GASNet::GASNet
	PRIVATE
		cinterface)
target_compile_definitions("${LIB_TARGET}"
	PRIVATE
		_GNU_SOURCE=1)
