find_package(SDL2 REQUIRED)

if(SDLRETRO_FRONTEND STREQUAL "SDL2")
    option(SDLRETRO_USE_OPENGL "Use OpenGL for rendering" OFF)
endif()

add_library(driver_sdl2 STATIC
    sdl2_impl.cpp
    include/sdl2_impl.h

    sdl2_input.cpp
    sdl2_input.h
    sdl2_video.cpp
    sdl2_video.h
    sdl2_audio.cpp
    sdl2_audio.h
    sdl2_ttf.cpp
    sdl2_ttf.h
    )

target_compile_definitions(driver_sdl2 PUBLIC SDL_MAIN_HANDLED)
target_include_directories(driver_sdl2 PUBLIC include)
target_link_libraries(driver_sdl2 driver_common SDL2::SDL2)


if(SDLRETRO_USE_OPENGL)
    target_compile_definitions(driver_sdl2 PRIVATE USE_OPENGL)
endif()
