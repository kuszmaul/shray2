# example fortran binaries
foreach(file
        cg
        )
	set(EXAMPLE_TARGET "${file}")

	add_executable("${EXAMPLE_TARGET}"
		"${file}.c")

	# TODO: use a proper find() for coarray stuff
	target_compile_options("${EXAMPLE_TARGET}"
		PRIVATE
			${PROJECT_FLAGS}
            -fopenmp
        )
	target_link_libraries("${EXAMPLE_TARGET}"
		PRIVATE
			${PROJECT_LINKER_FLAGS}
			-fopenmp)
endforeach(file)
