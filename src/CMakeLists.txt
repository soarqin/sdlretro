set(SDLRETRO_FRONTEND "SDL1" CACHE STRING "Select frontend (SDL1, SDL2)")

add_subdirectory(libretro)
add_subdirectory(drivers)
add_subdirectory(gui)

add_executable(sdlretro main.cpp miniz.c miniz.h)
target_link_libraries(sdlretro gui)

if(SDLRETRO_FRONTEND STREQUAL "SDL1")
    target_compile_definitions(sdlretro PRIVATE SDLRETRO_FRONTEND=1)
    target_link_libraries(sdlretro driver_sdl1)
endif()
if(SDLRETRO_FRONTEND STREQUAL "SDL2")
    target_compile_definitions(sdlretro PRIVATE SDLRETRO_FRONTEND=2)
    target_link_libraries(sdlretro driver_sdl2)
endif()
