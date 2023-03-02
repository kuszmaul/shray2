foreach(file
		bandwidth
		segfault)
	set(EXAMPLE_TARGET "${file}_mpi")

	add_executable("${EXAMPLE_TARGET}"
		"${file}.c")

	target_link_libraries("${EXAMPLE_TARGET}"
		PRIVATE
			MPI::MPI_C
			cinterface)
endforeach(file)
