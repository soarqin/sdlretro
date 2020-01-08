find_package(SDL REQUIRED)

add_library(driver_sdl1 STATIC
    sdl1_impl.cpp
    sdl1_input.cpp
    sdl1_video.cpp
    sdl1_font.cpp
    include/sdl1_impl.h
    include/sdl1_input.h
    include/sdl1_video.h
    include/sdl1_font.h

    sdl1_audio.cpp
    sdl1_audio.h
    )

target_compile_definitions(driver_sdl1 PUBLIC SDL_MAIN_HANDLED)
target_include_directories(driver_sdl1 PUBLIC include ${SDL_INCLUDE_DIR})
target_link_libraries(driver_sdl1 PUBLIC driver_common ${SDL_LIBRARY})
