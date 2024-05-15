# Add library for scheme parser
add_library(scheme_parser parser.cpp parser.h)

# Specify where this library's headers are and include dependencies
target_include_directories(scheme_parser PUBLIC 
${CMAKE_CURRENT_SOURCE_DIR}
${CMAKE_CURRENT_SOURCE_DIR}/../gc
${CMAKE_CURRENT_SOURCE_DIR}/../scheme-internals)
target_link_libraries(scheme_parser PUBLIC scheme_tokenizer)
