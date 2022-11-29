cmake_minimum_required (VERSION 3.10)
project(cpuid)

file(GLOB_RECURSE cpuid_sources src/*.cpp)
add_library(cpuid STATIC ${cpuid_sources})
add_library(steinwurf::cpuid ALIAS cpuid)

target_include_directories(cpuid PUBLIC include)
