# example fortran binaries
foreach(file
        blas
		matrix
        cg_single
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
            -fcheck=all
			-fcoarray=lib)
	target_link_libraries("${EXAMPLE_TARGET}"
		PRIVATE
			${PROJECT_LINKER_FLAGS}
			-lcaf_mpich
			#-lcaf_openmpi
			BLAS::BLAS
			-fopenmp)
endforeach(file)
