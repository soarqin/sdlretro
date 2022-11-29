cmake_minimum_required(VERSION 3.0)

if("${MODEL}" MATCHES "opendingux|gcw0|rg350")
    set(OPENDINGUX TRUE)
    set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/cmake/mips32-linux-gcc.cmake")
endif()

project(sdlretro)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

add_subdirectory(external)
add_subdirectory(src)
