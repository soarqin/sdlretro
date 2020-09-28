project(libgamecontrollerdb CXX)

add_library(libgamecontrollerdb STATIC
    src/gamecontrollerdb.cpp
    include/gamecontrollerdb.h)
add_library(libgamecontrollerdb::gamecontrollerdb ALIAS libgamecontrollerdb)
target_include_directories(libgamecontrollerdb PUBLIC include)
