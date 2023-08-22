if(MPI_BACKEND STREQUAL "mpich")
	set(LIBCAF_FLAGS "-lcaf_mpich")
else()
	set(LIBCAF_FLAGS "-lcaf_openmpi")
endif()

# example fortran binaries
foreach(file
		blas
		matrix
		cg
		dist_module
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
            -fexternal-blas
			-fcoarray=lib
        )
	target_link_libraries("${EXAMPLE_TARGET}"
		PRIVATE
			${PROJECT_LINKER_FLAGS}
			${LIBCAF_FLAGS}
			BLAS::BLAS
			-fopenmp)
endforeach(file)
