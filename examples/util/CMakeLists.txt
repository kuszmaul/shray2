add_library(util STATIC
	csr.c)

target_link_libraries(util
	PRIVATE
		cinterface)
