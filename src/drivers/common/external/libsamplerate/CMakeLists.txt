cmake_minimum_required(VERSION 3.1)
project(libsamplerate VERSION 0.1.9 LANGUAGES C)

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED TRUE)

include(ClipMode)

set(SAMPLERATE_SRC
	samplerate.c
	src_linear.c
	src_sinc.c
	src_zoh.c)

find_library(LIBSAMPLERATE_MATH_LIBRARY m)

# This will set CPU_CLIPS_NEGATIVE and CPU_CLIPS_POSITIVE
clip_mode()

configure_file(${PROJECT_SOURCE_DIR}/src_config.h.in ${CMAKE_CURRENT_BINARY_DIR}/src_config.h)

add_library(samplerate STATIC ${SAMPLERATE_SRC})

target_include_directories(samplerate PUBLIC . PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

if(LIBSAMPLERATE_MATH_LIBRARY)
    target_link_libraries(samplerate PUBLIC ${LIBSAMPLERATE_MATH_LIBRARY})
endif()
