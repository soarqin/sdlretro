find_package(Freetype REQUIRED)

add_library(driver_common STATIC
    # driver files
    buffered_audio.cpp
    driver_base.cpp
    input_base.cpp
    throttle.cpp
    ttf_font.cpp
    include/buffered_audio.h
    include/circular_buffer.h
    include/driver_base.h
    include/input_base.h
    include/throttle.h
    include/ttf_font.h
    include/video_base.h

    # logger
    logger.cpp
    include/logger.h
    )

target_include_directories(driver_common PRIVATE external ${FREETYPE_INCLUDE_DIRS})
target_include_directories(driver_common PUBLIC include)
target_link_libraries(driver_common PUBLIC libretro ${FREETYPE_LIBRARIES})
