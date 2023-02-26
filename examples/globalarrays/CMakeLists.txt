find_package(GlobalArrays REQUIRED)

foreach(file
		hello
        1dstencil
		matrix
		nbody
		spmv)
	set(EXAMPLE_TARGET "${file}_globalarrays")

	add_executable("${EXAMPLE_TARGET}"
		"${file}.c")

	target_link_libraries("${EXAMPLE_TARGET}"
		PRIVATE
			GlobalArrays::GlobalArrays
			BLAS::BLAS
			cinterface
			util)
endforeach(file)
