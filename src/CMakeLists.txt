include(language_standard)

file(READ shray.c FILE_CONTENTS_DEBUG)
set(FILE_CONTENTS_PROFILE "${FILE_CONTENTS_DEBUG}")
set(FILE_CONTENTS_NORMAL "${FILE_CONTENTS_DEBUG}")
foreach(fn
		ShrayInit
		ShrayMalloc
		ShrayStart
		ShrayEnd
		ShraySync
		ShrayFree
		ShrayReport
		ShrayRank
		ShraySize
		ShrayFinalize
	)
	string(REPLACE "${fn}" "${fn}_debug" FILE_CONTENTS_DEBUG
		"${FILE_CONTENTS_DEBUG}")
	string(REPLACE "${fn}" "${fn}_profile" FILE_CONTENTS_PROFILE
		"${FILE_CONTENTS_PROFILE}")
	string(REPLACE "${fn}" "${fn}_normal" FILE_CONTENTS_NORMAL
		"${FILE_CONTENTS_NORMAL}")
endforeach(fn)
file(WRITE "${PROJECT_BINARY_DIR}/shray_debug.c" "${FILE_CONTENTS_DEBUG}")
file(WRITE "${PROJECT_BINARY_DIR}/shray_profile.c" "${FILE_CONTENTS_PROFILE}")
file(WRITE "${PROJECT_BINARY_DIR}/shray_normal.c" "${FILE_CONTENTS_NORMAL}")

# build project library
set_source_files_properties("${PROJECT_BINARY_DIR}/shray_debug.c"
    PROPERTIES COMPILE_DEFINITIONS DEBUG)
set_source_files_properties("${PROJECT_BINARY_DIR}/shray_profile.c"
    PROPERTIES COMPILE_DEFINITIONS PROFILE)
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS shray.c)

set(LIB_TARGET "${PROJECT_NAME}")
add_library("${LIB_TARGET}" SHARED
	bitmap.c
	queue.c
	ringbuffer.c
	"${PROJECT_BINARY_DIR}/shray_debug.c"
	"${PROJECT_BINARY_DIR}/shray_normal.c"
	"${PROJECT_BINARY_DIR}/shray_profile.c")
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
