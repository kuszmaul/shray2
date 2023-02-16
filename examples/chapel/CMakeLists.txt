# Can't link to BLAS::BLASS since chapel has no separate link step.
# Instead, hack around it using BLAS_LIBRARIES.
get_filename_component(BLAS_TDIR ${BLAS_LIBRARIES} DIRECTORY)
get_filename_component(BLAS_TNAME ${BLAS_LIBRARIES} NAME)

foreach(file
		2dstencil
		1dstencil
		matrix
		nbody
		spmv)
	set(EXAMPLE_TARGET "${file}_chapel")

	add_executable("${EXAMPLE_TARGET}"
		"${file}.chpl")

	target_compile_options("${EXAMPLE_TARGET}"
		PRIVATE
			--fast
			-O
			-L${BLAS_TDIR}
			-l:${BLAS_TNAME})
endforeach(file)
