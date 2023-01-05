include(language_standard)

# build project library
foreach(suffix
		""
		_debug
		_profile)
	if(suffix STREQUAL "_debug")
		set(EXTRA_DEFINES DEBUG)
	elseif(suffix STREQUAL "_profile")
		set(EXTRA_DEFINES PROFILE)
	endif()

	set(LIB_TARGET "${PROJECT_NAME}${suffix}")
	add_library("${LIB_TARGET}" STATIC
		bitmap.c
		queue.c
		ringbuffer.c
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
		PUBLIC
			_GNU_SOURCE=1
		PRIVATE
			${EXTRA_DEFINES})
endforeach(suffix)
