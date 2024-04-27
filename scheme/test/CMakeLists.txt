# Find the Google Test package
find_package(GTest REQUIRED)

# Include Google Test's functionalities
include(GoogleTest)

# Test executable for control flow tests
add_executable(test_control_flow test_control_flow.cpp)
target_include_directories(test_control_flow PUBLIC "${PROJECT_SOURCE_DIR}/scheme")
target_link_libraries(test_control_flow GTest::GTest GTest::Main scheme scheme_parser scheme_tokenizer)
gtest_discover_tests(test_control_flow)

# Test executable for boolean tests
add_executable(test_boolean test_boolean.cpp)
target_include_directories(test_boolean PUBLIC "${PROJECT_SOURCE_DIR}/scheme")
target_link_libraries(test_boolean GTest::GTest GTest::Main scheme scheme_parser scheme_tokenizer)
gtest_discover_tests(test_boolean)

# Test executable for list tests
add_executable(test_list test_list.cpp)
target_include_directories(test_list PUBLIC "${PROJECT_SOURCE_DIR}/scheme")
target_link_libraries(test_list GTest::GTest GTest::Main scheme scheme_parser scheme_tokenizer)
gtest_discover_tests(test_list)

# Test executable for symbol tests
add_executable(test_symbol test_symbol.cpp)
target_include_directories(test_symbol PUBLIC "${PROJECT_SOURCE_DIR}/scheme")
target_link_libraries(test_symbol GTest::GTest GTest::Main scheme scheme_parser scheme_tokenizer)
gtest_discover_tests(test_symbol)

# Test executable for lambda tests
add_executable(test_lambda test_lambda.cpp)
target_include_directories(test_lambda PUBLIC "${PROJECT_SOURCE_DIR}/scheme")
target_link_libraries(test_lambda GTest::GTest GTest::Main scheme scheme_parser scheme_tokenizer)
gtest_discover_tests(test_lambda)

# Test executable for integer tests
add_executable(test_integer test_integer.cpp)
target_include_directories(test_integer PUBLIC "${PROJECT_SOURCE_DIR}/scheme")
target_link_libraries(test_integer GTest::GTest GTest::Main scheme scheme_parser scheme_tokenizer)
gtest_discover_tests(test_integer)

# Test executable for eval tests
add_executable(test_eval test_eval.cpp)
target_include_directories(test_eval PUBLIC "${PROJECT_SOURCE_DIR}/scheme")
target_link_libraries(test_eval GTest::GTest GTest::Main scheme scheme_parser scheme_tokenizer)
gtest_discover_tests(test_eval)
