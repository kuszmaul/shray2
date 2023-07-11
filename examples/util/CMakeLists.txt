add_library(util STATIC
	csr.c
	host.c)

target_link_libraries(util
	PRIVATE
		cinterface)

add_executable("makea" "makea.c")

target_link_libraries("makea"
    PRIVATE
        -fopenmp
        -O3
        -march=native
        -mtune=native
        -lm
)
