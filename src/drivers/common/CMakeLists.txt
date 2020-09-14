add_library(driver_common STATIC
    # driver files
    audio_base.cpp
    driver_base.cpp
    input_base.cpp
    throttle.cpp
    ttf_font_base.cpp
    video_base.cpp
    include/audio_base.h
    include/driver_base.h
    include/input_base.h
    include/throttle.h
    include/ttf_font_base.h
    include/video_base.h
    )

if(SDLRETRO_USE_STB_TRUETYPE)
    target_compile_definitions(driver_common PRIVATE USE_STB_TRUETYPE)
else()
    find_package(Freetype REQUIRED)
    target_include_directories(driver_common PRIVATE ${FREETYPE_INCLUDE_DIRS})
    target_link_libraries(driver_common ${FREETYPE_LIBRARIES})
endif()

target_include_directories(driver_common PRIVATE external)
target_include_directories(driver_common PUBLIC include)
target_link_libraries(driver_common libretro samplerate stb xxhash)
