foreach(file
	cg
	cg_comp
	)
	set(EXAMPLE_TARGET "${file}_omp")

	add_executable("${EXAMPLE_TARGET}"
		"${file}.c")

	target_link_libraries("${EXAMPLE_TARGET}"
		PRIVATE
            -lm
			cinterface)
endforeach(file)
