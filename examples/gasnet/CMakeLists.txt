foreach(file
        segfault)
	set(EXAMPLE_TARGET "${file}_gasnet")

	add_executable("${EXAMPLE_TARGET}"
		"${file}.c")

	target_link_libraries("${EXAMPLE_TARGET}"
		PRIVATE
		    GASNet::GASNet
			cinterface)
endforeach(file)
