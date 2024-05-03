add_library(scheme_parser gc.cpp parser.cpp)
target_include_directories(scheme_parser PUBLIC ${PROJECT_SOURCE_DIR}/scheme-parser)
target_link_libraries(scheme_parser scheme_tokenizer)
