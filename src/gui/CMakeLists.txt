add_library(gui STATIC
    ui_menu.cpp
    include/ui_menu.h
    )

target_include_directories(gui PUBLIC include)
target_link_libraries(gui PUBLIC driver_common)
