add_library(nlohmann_json INTERFACE)
target_include_directories(nlohmann_json INTERFACE .)
add_library(nlohmann::json ALIAS nlohmann_json)
