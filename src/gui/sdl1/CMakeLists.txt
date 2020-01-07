add_library(gui_sdl1 STATIC
    ui_menu.cpp
    include/ui_menu.h
    )

target_include_directories(gui_sdl1 PUBLIC include)
target_link_libraries(gui_sdl1 PUBLIC driver_sdl1)
