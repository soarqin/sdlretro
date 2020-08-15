add_library(gui STATIC
    menu_base.cpp
    menu_base.h
    sdl_menu.cpp
    sdl_menu.h
    ui_menu.cpp
    include/ui_menu.h
    )

target_include_directories(gui PUBLIC include)
target_link_libraries(gui driver_common)
