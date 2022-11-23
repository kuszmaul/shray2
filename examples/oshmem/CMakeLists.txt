include(language_standard)

# TODO: a bit of a hack, should fix this properly
set(CMAKE_C_COMPILER ${OSHCC})

# example binaries
foreach(file
		hello
		matrix
		nbody
		random
		2dstencil)
	set(EXAMPLE_TARGET "${file}_oshmem")

	add_executable("${EXAMPLE_TARGET}"
		"${file}.c")

	target_link_libraries("${EXAMPLE_TARGET}"
		PRIVATE
			BLAS::BLAS
			cinterface
			m)
endforeach(file)
