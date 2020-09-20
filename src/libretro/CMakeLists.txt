add_library(libretro STATIC
    # config
    cfg.cpp
    i18n.cpp
    variables.cpp
    include/cfg.h
    include/i18n.h
    include/variables.h

    # utils
    util.cpp
    include/util.h
    dlfcn_compat.h # dlfcn compatible header for win32

    # vfs
    vfs_win32.cpp
    vfs_unix.cpp

    # cores
    core.c
    core_manager.cpp
    include/core.h
    include/core_manager.h
    include/libretro.h
    )

if(WIN32)
    target_compile_definitions(libretro PRIVATE VFS_WIN32)
elseif(UNIX)
    target_compile_definitions(libretro PRIVATE VFS_UNIX)
endif()

target_include_directories(libretro PUBLIC include)
target_link_libraries(libretro spdlog::spdlog nlohmann::json xxhash::xxhash)
if(WIN32)
    target_link_libraries(libretro shlwapi)
else()
    target_link_libraries(libretro dl)
endif()
