# example binaries
foreach(suffix
		""
		_debug
		_profile)
	if(suffix STREQUAL "_debug")
		set(EXTRA_DEFINES DEBUG)
	elseif(suffix STREQUAL "_profile")
		set(EXTRA_DEFINES PROFILE)
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
			spmv
			spmv_mt)
		set(SHRAY_TARGET "${PROJECT_NAME}${suffix}")
		set(EXAMPLE_TARGET "${file}${suffix}_shray")

		add_executable("${EXAMPLE_TARGET}"
			"${file}.c")

		target_link_libraries("${EXAMPLE_TARGET}"
			PRIVATE
				${SHRAY_TARGET}
				BLAS::BLAS
				cinterface
				util)
		target_compile_definitions("${EXAMPLE_TARGET}"
			PRIVATE
				_GNU_SOURCE=1
				${EXTRA_DEFINES})
	endforeach(file)
endforeach(suffix)
