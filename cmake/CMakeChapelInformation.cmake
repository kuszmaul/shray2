# Chapel's compilation structure does not match CMake's expected structure,
# we simply leave the linking step empty and assume COMPILE_OBJECT creates the
# final executable.
if(NOT CMAKE_Chapel_COMPILE_OBJECT)
    set(CMAKE_Chapel_COMPILE_OBJECT "<CMAKE_Chapel_COMPILER> <FLAGS> <SOURCE> -o <TARGET>")
endif()
if(NOT CMAKE_Chapel_LINK_EXECUTABLE)
    set(CMAKE_Chapel_LINK_EXECUTABLE "")
endif()
set(CMAKE_Chapel_INFORMATION_LOADED 1)
