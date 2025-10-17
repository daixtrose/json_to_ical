cmake_minimum_required(VERSION 3.14)
project(JsonToIcal VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include FetchContent module
include(FetchContent)

# Fetch PEGTL
FetchContent_Declare(
    pegtl
    GIT_REPOSITORY https://github.com/taocpp/PEGTL.git
    GIT_TAG 3.2.7
    GIT_SHALLOW TRUE
)

# Fetch libical
FetchContent_Declare(
    libical
    GIT_REPOSITORY https://github.com/libical/libical.git
    GIT_TAG v3.0.18
    GIT_SHALLOW TRUE
)

# Configure libical options before making it available
set(ICAL_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(LIBICAL_BUILD_TESTING OFF CACHE BOOL "" FORCE)
set(ICAL_GLIB OFF CACHE BOOL "" FORCE)
set(WITH_CXX_BINDINGS OFF CACHE BOOL "" FORCE)
set(LIBICAL_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

# Make dependencies available
FetchContent_MakeAvailable(pegtl libical)

# Add the executable
add_executable(json_to_ical
    src/main.cpp
    src/json_to_ical.cpp
)

# Link libraries
target_link_libraries(json_to_ical PRIVATE 
    taocpp::pegtl
    ical
)

# Include directories
target_include_directories(json_to_ical PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Installation
install(TARGETS json_to_ical DESTINATION bin)
