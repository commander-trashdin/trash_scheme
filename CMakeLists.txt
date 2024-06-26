cmake_minimum_required(VERSION 3.10)
project(SchemeInterpreter)

# Specify C++ Standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fexceptions")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_BUILD_TYPE Debug)

add_compile_options(-fsanitize=address -g -O0)
add_link_options(-fsanitize=address)

# Include subdirectories
add_subdirectory(scheme-tokenizer)  # This should come before any dependencies that require it
add_subdirectory(scheme-parser)
add_subdirectory(scheme)  # Assuming 'scheme' directory contains main application and depends on both parser and tokenizer

# Optionally enable testing globally if all sub-projects include tests
enable_testing()
