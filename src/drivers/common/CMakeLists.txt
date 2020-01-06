find_package(Freetype REQUIRED)

add_library(driver_common STATIC
    # config
    cfg.cpp
    include/cfg.h

    # core files
    core_manager.cpp
    include/core_manager.h

    # driver files
    buffered_audio.cpp
    driver_base.cpp
    input_base.cpp
    menu_base.cpp
    throttle.cpp
    ttf_font.cpp
    include/buffered_audio.h
    include/circular_buffer.h
    include/driver_base.h
    include/input_base.h
    include/menu_base.h
    include/throttle.h
    include/ttf_font.h
    include/video_base.h

    # utils
    util.cpp
    include/util.h
    logger.cpp
    include/logger.h

    # external libs
    external/json.hpp
    )

target_include_directories(driver_common PRIVATE external ${FREETYPE_INCLUDE_DIRS})
target_include_directories(driver_common PUBLIC include)
target_link_libraries(driver_common PUBLIC libretro ${FREETYPE_LIBRARIES})
if(WIN32)
    target_link_libraries(driver_common PUBLIC shlwapi)
endif()
