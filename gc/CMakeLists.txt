# Add library for garbage collection
add_library(gc gc.cpp gc.h getconst.h)

# Specify where this library's headers are
target_include_directories(gc PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(gc PUBLIC scheme_internals)