add_library(libretro STATIC
    # config
    cfg.cpp
    variables.cpp
    include/cfg.h
    include/variables.h

    # utils
    util.cpp
    include/util.h

    # cores
    core.c
    core_manager.cpp
    include/core.h
    include/core_manager.h
    include/libretro.h

    # external libs
    external/json.hpp
    )
target_include_directories(libretro PRIVATE external)
target_include_directories(libretro PUBLIC include)
if(WIN32)
    target_link_libraries(libretro PUBLIC shlwapi)
else()
    target_link_libraries(libretro PUBLIC dl)
endif()
