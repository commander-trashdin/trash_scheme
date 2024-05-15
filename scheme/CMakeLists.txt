# Main executable
# Main executable
add_executable(scheme main.cpp scheme.cpp)

# Include directories
target_include_directories(scheme PRIVATE 
    ${CMAKE_SOURCE_DIR}/gc
    ${CMAKE_SOURCE_DIR}/scheme-tokenizer
    ${CMAKE_SOURCE_DIR}/scheme-parser
    ${CMAKE_SOURCE_DIR}/scheme-internals
)

# Link libraries
target_link_libraries(scheme PRIVATE 
    gc
    scheme_tokenizer
    scheme_parser
    scheme_internals
)
