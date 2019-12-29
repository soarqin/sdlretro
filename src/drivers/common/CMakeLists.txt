add_library(driver_common STATIC
    driver_base.cpp
    input.cpp
    throttle.cpp
    util.cpp
    include/driver_base.h
    include/input.h
    include/throttle.h
    include/util.h

    buffered_audio.cpp
    buffered_audio.h
    circular_buffer.h
    logger.cpp
    logger.h

    )

target_include_directories(driver_common PUBLIC include)
target_link_libraries(driver_common PUBLIC libretro)
