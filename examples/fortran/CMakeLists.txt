# example fortran binaries
foreach(file
		2dstencil
		matrix
		nbody
		nbodyUnblocked
		random
        1dstencil
		spmv)
	set(EXAMPLE_TARGET "${file}_fortran")

	add_executable("${EXAMPLE_TARGET}"
		"${file}.f90")

	# TODO: use a proper find() for coarray stuff
	target_compile_options("${EXAMPLE_TARGET}"
		PRIVATE
			${PROJECT_FLAGS}
            -cpp
			-fcoarray=lib)
	target_link_libraries("${EXAMPLE_TARGET}"
		PRIVATE
			${PROJECT_LINKER_FLAGS}
			-lcaf_openmpi)
endforeach(file)
