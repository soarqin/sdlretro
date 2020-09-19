find_package(SDL2 REQUIRED)

option(SDLRETRO_USE_OPENGL "Use OpenGL for rendering" OFF)
option(SDLRETRO_USE_OPENGL_ES "Use OpenGL ES for rendering" OFF)

if (SDLRETRO_USE_OPENGL)
    set(video_src
        sdl2_video_ogl.cpp
        sdl2_video_ogl.h
        sdl2_ttf_ogl.cpp
        sdl2_ttf_ogl.h)
else()
    set(video_src
        sdl2_video.cpp
        sdl2_video.h
        sdl2_ttf.cpp
        sdl2_ttf.h)
endif()

add_library(driver_sdl2 STATIC
    sdl2_impl.cpp
    include/sdl2_impl.h

    sdl2_input.cpp
    sdl2_input.h
    ${video_src}
    sdl2_audio.cpp
    sdl2_audio.h
    )

target_compile_definitions(driver_sdl2 PUBLIC SDL_MAIN_HANDLED)
target_include_directories(driver_sdl2 PUBLIC include ${SDL2_INCLUDE_DIRS})
target_link_libraries(driver_sdl2 driver_common ${SDL2_LIBRARIES})

if(SDLRETRO_USE_OPENGL_ES)
    target_compile_definitions(driver_sdl2 PRIVATE USE_OPENGL USE_GLES)
    target_link_libraries(driver_sdl2 glad)
elseif(SDLRETRO_USE_OPENGL)
    target_compile_definitions(driver_sdl2 PRIVATE USE_OPENGL)
    target_link_libraries(driver_sdl2 glad)
endif()
