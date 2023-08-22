# Can't link to BLAS::BLASS since chapel has no separate link step.
# Instead, hack around it using BLAS_LIBRARIES.
list(LENGTH BLAS_LIBRARIES blas_count)
if(${blas_count} EQUAL 1)
       set(BLAS_LIB_ACTUAL ${BLAS_LIBRARIES})
else()
       list(GET BLAS_LIBRARIES 0 BLAS_LIB_ACTUAL)
endif()
get_filename_component(BLAS_TDIR ${BLAS_LIB_ACTUAL} DIRECTORY)
get_filename_component(BLAS_TNAME ${BLAS_LIB_ACTUAL} NAME)

foreach(file
		cg
		1dstencil
		matrix
		monopoly
	 )
	set(EXAMPLE_TARGET "${file}_chapel")

	add_executable("${EXAMPLE_TARGET}"
		"${file}.chpl")

	target_compile_options("${EXAMPLE_TARGET}"
		PRIVATE
			--ccflags
			-Ofast
			--fast
			--no-ieee-float
			-O
			-L${BLAS_TDIR}
			-l:${BLAS_TNAME})
endforeach(file)
