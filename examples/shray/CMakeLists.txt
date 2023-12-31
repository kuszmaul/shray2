# example binaries
foreach(suffix
		_normal
		_debug
		_profile)
	if(suffix STREQUAL "_debug")
		set(EXTRA_DEFINES SHRAY_DEBUG)
	elseif(suffix STREQUAL "_profile")
		set(EXTRA_DEFINES SHRAY_PROFILE)
	endif()

	foreach(file
			cg
			1dstencil
			1dstencil_mt
			bandwidth
			matrixAuto
			monopoly
			monopoly_mt
			reduce
			sync
			spmv
			spmv_mt
		)
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
