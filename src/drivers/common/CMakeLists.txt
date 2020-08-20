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
    )

if(USE_STB_TRUETYPE)
    target_compile_definitions(driver_common PRIVATE USE_STB_TRUETYPE)
else()
    find_package(Freetype REQUIRED)
    target_include_directories(driver_common PRIVATE ${FREETYPE_INCLUDE_DIRS})
    target_link_libraries(driver_common ${FREETYPE_LIBRARIES})
endif()

target_include_directories(driver_common PRIVATE external)
target_include_directories(driver_common PUBLIC include)
target_link_libraries(driver_common libretro samplerate stb)
