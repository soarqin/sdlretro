add_library(util STATIC
    logger.cpp include/logger.h)
target_include_directories(util PUBLIC include)
target_link_libraries(util PUBLIC fmt::fmt)
