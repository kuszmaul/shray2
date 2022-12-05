find_package(BLAS REQUIRED)

# example binaries
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

	foreach(file
			2dstencil
			2dstencilAuto
			bandwidth
			matrix
			matrixAuto
			matrix_mt
			nbody
			nbodyAuto
			nbody_mt
			random
			reduce
			segfault
			sync
			spmv)
		set(SHRAY_TARGET "${PROJECT_NAME}${suffix}")
		set(EXAMPLE_TARGET "${file}${suffix}")

		add_executable("${EXAMPLE_TARGET}"
			"${file}.c")

		target_link_libraries("${EXAMPLE_TARGET}"
			PRIVATE
				${SHRAY_TARGET}
				BLAS::BLAS
				cinterface)
		target_compile_definitions("${EXAMPLE_TARGET}"
			PRIVATE
				${EXTRA_DEFINES})
	endforeach(file)
endforeach(suffix)
