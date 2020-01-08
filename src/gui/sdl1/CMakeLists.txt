add_library(gui_sdl1 STATIC
    sdl1_menu.cpp
    ui_menu.cpp
    include/sdl1_menu.h
    include/ui_menu.h
    )

target_include_directories(gui_sdl1 PUBLIC include)
target_link_libraries(gui_sdl1 PUBLIC gui_common driver_sdl1)
