if(OPENDINGUX)
    set(DEFAULT_FRONTEND SDL1)
else()
    set(DEFAULT_FRONTEND SDL2)
endif()
if(MSVC)
    set(DEFAULT_STB_TRUETYPE ON)
else()
    set(DEFAULT_STB_TRUETYPE OFF)
endif()

if(MSVC)
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

# add -Wno-psabi to strip abi warnings for gcc 6-8 in nlohmann json
if(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} -Wno-psabi")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-psabi")
else()
    set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} /utf-8")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /utf-8")
endif()

set(SDLRETRO_FRONTEND "${DEFAULT_FRONTEND}" CACHE STRING "Select frontend (SDL1, SDL2)")
option(SDLRETRO_USE_STB_TRUETYPE "Use stb truetype (aka build w/o FreeType)" ${DEFAULT_STB_TRUETYPE})

add_subdirectory(libretro)
add_subdirectory(drivers)
add_subdirectory(gui)
add_subdirectory(external)

add_executable(sdlretro main.cpp)
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
    target_link_libraries(sdlretro wingetopt)
endif()
