cmake_minimum_required(VERSION 3.16)

project(REBTChecker
    VERSION 0.1.0
    LANGUAGES CXX
)

# Options
option(BUILD_TESTS "Build tests" OFF)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Treat warnings as errors for your code
if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()

# Use FetchContent to bring in nlohmann::json (header-only)
include(FetchContent)

FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG        v3.11.3  # or any stable tag you like
)

FetchContent_MakeAvailable(nlohmann_json)

# Add src directory
add_subdirectory(src)

# (Optional) enable tests later
if (BUILD_TESTS)
    enable_testing()
    # add_subdirectory(tests)
endif()
