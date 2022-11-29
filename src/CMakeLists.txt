if(OPENDINGUX)
    set(DEFAULT_FRONTEND SDL1)
else()
    set(DEFAULT_FRONTEND SDL2)
endif()
if(WIN32)
    set(DEFAULT_CORE_DOWNLOADER OFF)
else()
    set(DEFAULT_CORE_DOWNLOADER ON)
endif()

if(WIN32)
    # use stb_truetype by default on windows
    set(DEFAULT_STB_TRUETYPE ON)
    set(DEFAULT_STATIC_CRT ON)
else()
    # FreeType can be easily installed on Linux/macOS so we use it as default TTF lib
    set(DEFAULT_STB_TRUETYPE OFF)
    set(DEFAULT_STATIC_CRT OFF)
endif()

option(SDLRETRO_USE_STATIC_CRT "Use static C Runtime" ${DEFAULT_STATIC_CRT})
set(SDLRETRO_FRONTEND "${DEFAULT_FRONTEND}" CACHE STRING "Select frontend (SDL1, SDL2)")
option(SDLRETRO_USE_STB_TRUETYPE "Use stb truetype (aka build w/o FreeType)" ${DEFAULT_STB_TRUETYPE})
option(SDLRETRO_CORE_DOWNLOADER "Enable core downloader" ${DEFAULT_CORE_DOWNLOADER})

if(SDLRETRO_CORE_DOWNLOADER)
    find_package(CURL REQUIRED)
endif()

if(SDLRETRO_USE_STATIC_CRT)
    if(CMAKE_COMPILER_IS_GNUCXX)
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static -static-libgcc -static-libstdc++")
    elseif(MSVC)
        set(CompilerFlags
            CMAKE_CXX_FLAGS
            CMAKE_CXX_FLAGS_DEBUG
            CMAKE_CXX_FLAGS_RELEASE
            CMAKE_CXX_FLAGS_MINSIZEREL
            CMAKE_CXX_FLAGS_RELWITHDEBINFO
            CMAKE_C_FLAGS
            CMAKE_C_FLAGS_DEBUG
            CMAKE_C_FLAGS_RELEASE
            CMAKE_C_FLAGS_MINSIZEREL
            CMAKE_C_FLAGS_RELWITHDEBINFO
            )
        foreach(CompilerFlag ${CompilerFlags})
            string(REPLACE "/MD" "/MT" ${CompilerFlag} "${${CompilerFlag}}")
            set(${CompilerFlag} "${${CompilerFlag}}" CACHE STRING "msvc compiler flags" FORCE)
        endforeach()
    endif()
endif()

if(CMAKE_COMPILER_IS_GNUCXX)
    # add -Wno-psabi to strip abi warnings for gcc 6-8 in nlohmann json
    set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} -Wno-psabi")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-psabi")
elseif(MSVC)
    # add /utf-8 to make VC recognizing utf-8 encoded strings
    set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} /utf-8")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /utf-8")
endif()

add_subdirectory(util)
add_subdirectory(libretro)
add_subdirectory(drivers)
add_subdirectory(gui)

if((NOT WIN32) OR CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_executable(sdlretro main.cpp)
else()
    add_executable(sdlretro WIN32 main.cpp)
endif()
target_link_libraries(sdlretro gui miniz)

if(SDLRETRO_FRONTEND STREQUAL "SDL1")
    target_compile_definitions(sdlretro PRIVATE SDLRETRO_FRONTEND=1)
    target_link_libraries(sdlretro driver_sdl1)
elseif(SDLRETRO_FRONTEND STREQUAL "SDL2")
    target_compile_definitions(sdlretro PRIVATE SDLRETRO_FRONTEND=2)
    target_link_libraries(sdlretro driver_sdl2)
else()
    message(FATAL_ERROR SDLRETRO_FRONTEND must be SDL1 or SDL2)
endif()

if(MSVC)
    # implementation of getopt for MSVC
    target_link_libraries(sdlretro wingetopt)
endif()

set_target_properties(sdlretro PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
