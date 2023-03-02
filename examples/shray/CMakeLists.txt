# example binaries
foreach(suffix
		_normal
		_debug
		_profile)
	if(suffix STREQUAL "_debug")
		set(EXTRA_DEFINES DEBUG)
	elseif(suffix STREQUAL "_profile")
		set(EXTRA_DEFINES PROFILE)
	endif()

	foreach(file
			1dstencil
			1dstencil_mt
			2dstencil
			2dstencilAuto
			bandwidth
			matrix
            broadcast
			matrixAuto
			matrix_mt
            monopoly
			gasnet
			nbody
			nbodyAuto
			nbody_mt
			random
			reduce
			segfault
            segfault_prefetch
			sync
            synctest
			spmv
			spmv_mt)
		set(SHRAY_TARGET "${PROJECT_NAME}")
		set(EXAMPLE_TARGET "${file}${suffix}_shray")

		add_executable("${EXAMPLE_TARGET}"
			"${file}.c")

		if(GASNet_CONDUIT STREQUAL "udp")
			set_source_files_properties("${file}.c" PROPERTIES LANGUAGE CXX)
		endif()

		target_link_libraries("${EXAMPLE_TARGET}"
			PRIVATE
				${SHRAY_TARGET}
				BLAS::BLAS
                GASNet::GASNet
				cinterface
				util)
		target_compile_definitions("${EXAMPLE_TARGET}"
			PRIVATE
				_GNU_SOURCE=1
				${EXTRA_DEFINES})
	endforeach(file)
endforeach(suffix)
