if(SDLRETRO_CORE_DOWNLOADER)
    message(STATUS "Found libcurl version: ${CURL_VERSION_STRING}")
    set(EXTRA_SRC downloader.cpp include/downloader.h)
endif()

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
    perf.cpp
    include/core.h
    include/core_manager.h
    include/perf.h
    include/libretro.h

    ${EXTRA_SRC}
    )

if(WIN32)
    target_compile_definitions(libretro PRIVATE VFS_WIN32)
elseif(UNIX)
    target_compile_definitions(libretro PRIVATE VFS_UNIX)
endif()

if(SDLRETRO_CORE_DOWNLOADER)
    target_compile_definitions(libretro PUBLIC CORE_DOWNLOADER)
endif()

target_include_directories(libretro PUBLIC include)

target_link_libraries(libretro
    PUBLIC  spdlog::spdlog nlohmann::json
    PRIVATE xxhash::xxhash steinwurf::cpuid)
if(CURL_FOUND)
    target_link_libraries(libretro PRIVATE CURL::libcurl)
endif()
if(WIN32)
    target_link_libraries(libretro PUBLIC shlwapi)
else()
    target_link_libraries(libretro PUBLIC dl)
endif()
