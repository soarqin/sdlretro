option(SDLRETRO_USE_SDL1 "Use SDL1 frontend" ON)

add_subdirectory(libretro)
add_subdirectory(drivers)
add_subdirectory(gui)

add_executable(sdlretro main.cpp)

if(SDLRETRO_USE_SDL1)
    target_link_libraries(sdlretro driver_sdl1 gui_sdl1)
endif()
