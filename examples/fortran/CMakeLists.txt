# example fortran binaries
foreach(file
		matrix
        monopoly
        monopoly_mt
        1dstencil
        1dstencil_mt)
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
            -lcaf_openmpi
            -fopenmp)
endforeach(file)
