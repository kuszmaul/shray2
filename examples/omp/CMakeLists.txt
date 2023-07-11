foreach(file
	cg
	)
	set(EXAMPLE_TARGET "${file}_omp")

	add_executable("${EXAMPLE_TARGET}"
		"${file}.c")

	target_link_libraries("${EXAMPLE_TARGET}"
		PRIVATE
			cinterface)
endforeach(file)
