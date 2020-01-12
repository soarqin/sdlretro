find_package(SDL REQUIRED)

add_library(driver_sdl1 STATIC
    sdl1_impl.cpp
    sdl1_input.cpp
    sdl1_video.cpp
    include/sdl1_impl.h
    include/sdl1_input.h
    include/sdl1_video.h

    sdl1_audio.cpp
    sdl1_audio.h
    sdl1_font.cpp
    sdl1_font.h
    )

target_compile_definitions(driver_sdl1 PUBLIC SDL_MAIN_HANDLED)
target_include_directories(driver_sdl1 PUBLIC include ${SDL_INCLUDE_DIR})
list(REMOVE_ITEM SDL_LIBRARY "${SDLMAIN_LIBRARY}")
target_link_libraries(driver_sdl1 driver_common ${SDL_LIBRARY})
