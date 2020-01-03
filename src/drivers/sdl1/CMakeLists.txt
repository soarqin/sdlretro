find_package(SDL REQUIRED)

add_library(driver_sdl1 STATIC
    sdl1_impl.cpp
    include/sdl1_impl.h

    sdl1_audio.cpp
    sdl1_audio.h
    sdl1_video.cpp
    sdl1_video.h
    sdl1_input.cpp
    sdl1_input.h
    sdl1_font.cpp
    sdl1_font.h
    )

target_compile_definitions(driver_sdl1 PUBLIC SDL_MAIN_HANDLED)
target_include_directories(driver_sdl1 PUBLIC include)
target_include_directories(driver_sdl1 PRIVATE ${SDL_INCLUDE_DIR})
target_link_libraries(driver_sdl1 PUBLIC driver_common ${SDL_LIBRARY})
