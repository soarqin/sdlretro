add_subdirectory(libretro)
add_subdirectory(drivers)
add_subdirectory(gui)

add_executable(sdlretro main.cpp)

target_link_libraries(sdlretro driver_sdl1 gui)
