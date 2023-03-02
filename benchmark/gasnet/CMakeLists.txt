foreach(file
		large
		small)
	set(EXAMPLE_TARGET "${file}_gasnet")

	add_executable("${EXAMPLE_TARGET}"
		"${file}.c")

	if(GASNet_CONDUIT STREQUAL "udp")
		set_source_files_properties("${file}.c" PROPERTIES LANGUAGE CXX)
	endif()

	target_link_libraries("${EXAMPLE_TARGET}"
		PRIVATE
			GASNet::GASNet
			cinterface)
endforeach(file)
