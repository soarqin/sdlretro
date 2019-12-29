add_subdirectory(libretro)
add_subdirectory(drivers)

add_executable(sdlretro
    main.cpp
    )

target_link_libraries(sdlretro driver_sdl1)
