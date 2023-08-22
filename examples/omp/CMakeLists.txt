foreach(file
	cg
	cg_comp
	)
	set(EXAMPLE_TARGET "${file}_c")

	add_executable("${EXAMPLE_TARGET}"
		"${file}.c")

	target_link_libraries("${EXAMPLE_TARGET}"
		PRIVATE
            -lm
			cinterface)
endforeach(file)

foreach(file
    cg_omp
	)
	set(EXAMPLE_TARGET "${file}_fortran")

	add_executable("${EXAMPLE_TARGET}"
		"${file}.f90")

	target_compile_options("${EXAMPLE_TARGET}"
		PRIVATE
			${PROJECT_FLAGS}
			-cpp
            -fcheck=all
        )

	target_link_libraries("${EXAMPLE_TARGET}"
		PRIVATE
            -fopenmp
			)
endforeach(file)
