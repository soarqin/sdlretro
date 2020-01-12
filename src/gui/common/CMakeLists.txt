add_library(gui_common STATIC
    menu_base.cpp
    include/menu_base.h
    )

target_include_directories(gui_common PUBLIC include)
target_link_libraries(gui_common driver_common)
