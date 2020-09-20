add_library(xxhash xxhash.c)
target_include_directories(xxhash PUBLIC .)
add_library(xxhash::xxhash ALIAS xxhash)
