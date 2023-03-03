add_library(util STATIC
	csr.c
	host.c)

target_link_libraries(util
	PRIVATE
		cinterface)
