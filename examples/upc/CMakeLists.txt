foreach(file
		matrix
		1dstencil
		monopoly)
	set(EXAMPLE_TARGET "${file}_upc")

	add_executable("${EXAMPLE_TARGET}"
		"${file}.upc")

	# UPC also uses gasnet so make sure we use the same conduit.
	target_compile_options("${EXAMPLE_TARGET}"
		PRIVATE
			-network=${GASNet_CONDUIT})
	target_link_options("${EXAMPLE_TARGET}"
		PRIVATE
			-network=${GASNet_CONDUIT})
	target_link_libraries("${EXAMPLE_TARGET}"
		PRIVATE
			BLAS::BLAS
			util)
endforeach(file)
