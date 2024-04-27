# Main executable
add_executable(scheme main.cpp scheme.cpp)
set_target_properties(scheme PROPERTIES ENABLE_EXPORTS ON)
target_include_directories(scheme PUBLIC "${PROJECT_SOURCE_DIR}")


# Link with parser and tokenizer libraries
target_link_libraries(scheme scheme_parser)
target_link_libraries(scheme scheme_tokenizer)

# Tests
add_subdirectory(test)
