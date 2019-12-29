add_library(libretro STATIC
    include/libretro.h
    core.c
    include/core.h
    )
target_include_directories(libretro PUBLIC include)
