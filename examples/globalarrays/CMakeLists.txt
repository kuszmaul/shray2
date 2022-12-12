find_package(GlobalArrays REQUIRED)

foreach(file
		hello
		matrix
		nbody
		spmv)
	set(SHRAY_TARGET "${PROJECT_NAME}")
	set(EXAMPLE_TARGET "${file}_globalarrays")

	add_executable("${EXAMPLE_TARGET}"
		"${file}.c")

	target_link_libraries("${EXAMPLE_TARGET}"
		PRIVATE
			GlobalArrays::GlobalArrays
			cinterface
			util)
endforeach(file)
