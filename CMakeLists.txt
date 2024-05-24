cmake_minimum_required(VERSION 3.10)
project(SchemeInterpreter)

# Set C++ standard requirements
set(CMAKE_C_COMPILER "/usr/bin/clang-18")
set(CMAKE_CXX_COMPILER "/usr/bin/clang++-18")
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_BUILD_TYPE Debug)

# Compiler options for development
add_compile_options(-g -fno-inline -O0 -fsanitize=address)
add_link_options(-fsanitize=address)

# Adding subdirectories
add_subdirectory(gc)
add_subdirectory(scheme-internals)
add_subdirectory(scheme-tokenizer)
add_subdirectory(scheme-parser)
add_subdirectory(scheme)

# Assuming 'scheme' subdirectory defines 'SchemeInterpreter' executable
# Set include directories here, assuming 'SchemeInterpreter' is the target name defined in the 'scheme' subdirectory


# enable_testing()
