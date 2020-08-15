option(SDLRETRO_USE_SDL1 "Use SDL1 frontend" ON)

add_subdirectory(libretro)
add_subdirectory(drivers)
add_subdirectory(gui)

add_executable(sdlretro main.cpp miniz.c miniz.h)
target_link_libraries(sdlretro gui)

if(SDLRETRO_USE_SDL1)
    target_compile_definitions(sdlretro PRIVATE USE_SDL1)
    target_link_libraries(sdlretro driver_sdl1)
endif()
if(SDLRETRO_USE_SDL2)
    target_compile_definitions(sdlretro PRIVATE USE_SDL2)
    target_link_libraries(sdlretro driver_sdl2)
endif()
