cmake_minimum_required(VERSION 3.10)
project(JsonToIcal VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required libraries
find_package(nlohmann_json 3.2.0 REQUIRED)
find_package(LibIcal REQUIRED)

# Add the executable
add_executable(json_to_ical
    src/main.cpp
    src/json_to_ical.cpp
)

# Link libraries
target_link_libraries(json_to_ical PRIVATE
    nlohmann_json::nlohmann_json
    ${LibIcal_LIBRARIES}
)

# Include directories
target_include_directories(json_to_ical PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Installation
install(TARGETS json_to_ical DESTINATION bin)
