project(stb C)

add_library(stb STATIC stb.c)
if(SDLRETRO_USE_STB_TRUETYPE)
    target_compile_definitions(stb PRIVATE USE_STB_TRUETYPE)
endif()
target_include_directories(stb PUBLIC .)
