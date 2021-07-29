if (TEST_SOLUTION)
  include_directories(../private/scheme-tokenizer)
else()
  include_directories(../scheme-tokenizer)
endif()

add_catch(test_scheme_parser
  parser_test.cpp
  SOLUTION_SRCS parser.cpp)
