foreach(file
		bitmap
	)
	set(TEST_FILE "test_${file}")
	add_executable(${TEST_FILE}
		"${file}.c")
	target_link_libraries(${TEST_FILE}
		PRIVATE
			${PROJECT_NAME}
			cinterface
		)
	add_test(${file} ${TEST_FILE})
endforeach()
