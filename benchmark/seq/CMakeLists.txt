foreach(file
		bandwidth
		free
		latency
		mprotect
		remap
		segv)
	set(EXAMPLE_TARGET "${file}_seq")

	add_executable("${EXAMPLE_TARGET}"
		"${file}.c")

	target_link_libraries("${EXAMPLE_TARGET}"
		PRIVATE
			cinterface)
endforeach(file)
