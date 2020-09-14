add_subdirectory(json)
add_subdirectory(libsamplerate)
add_subdirectory(miniz)
add_subdirectory(spdlog)
add_subdirectory(stb)
add_subdirectory(glad)

add_library(xxhash xxHash/xxhash.c)
target_include_directories(xxhash PUBLIC xxHash)
