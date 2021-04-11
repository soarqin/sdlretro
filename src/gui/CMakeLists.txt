add_library(gui STATIC
    gui_base.h
    elem_base.cpp
    elem_base.h
    sdl_elem.cpp
    sdl_elem.h
    menu_base.cpp
    menu_base.h
    sdl_menu.cpp
    sdl_menu.h
    ui_host.cpp
    include/ui_host.h
    )
if(SDLRETRO_FRONTEND STREQUAL "SDL1")
    target_compile_definitions(gui PRIVATE SDLRETRO_FRONTEND=1)
endif()
if(SDLRETRO_FRONTEND STREQUAL "SDL2")
    target_compile_definitions(gui PRIVATE SDLRETRO_FRONTEND=2)
endif()

target_include_directories(gui PUBLIC include)
target_link_libraries(gui driver_common)
