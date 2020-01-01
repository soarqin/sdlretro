add_library(driver_common STATIC
    buffered_audio.cpp
    driver_base.cpp
    input.cpp
    throttle.cpp
    util.cpp
    include/buffered_audio.h
    include/circular_buffer.h
    include/driver_base.h
    include/input.h
    include/throttle.h
    include/util.h
    include/video_base.h

    logger.cpp
    logger.h
    )

target_include_directories(driver_common PUBLIC include)
target_link_libraries(driver_common PUBLIC libretro)
