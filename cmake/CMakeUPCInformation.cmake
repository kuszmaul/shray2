if(NOT CMAKE_UPC_COMPILE_OBJECT)
    set(CMAKE_UPC_COMPILE_OBJECT "<CMAKE_UPC_COMPILER> <DEFINES> <INCLUDES> <FLAGS> -o <OBJECT> -c <SOURCE>")
endif()
if(NOT CMAKE_UPC_LINK_EXECUTABLE)
    set(CMAKE_UPC_LINK_EXECUTABLE "<CMAKE_UPC_COMPILER> -o <TARGET> <OBJECTS> <CMAKE_C_LINK_FLAGS> <LINK_FLAGS> <LINK_LIBRARIES>")
endif()
set(CMAKE_UPC_INFORMATION_LOADED 1)