#include "scheme_test.h"
#include <gtest/gtest.h>
#include <typeinfo>

void ReadFull(const std::string &str) {
  std::stringstream ss{str};
  Tokenizer tokenizer{&ss};

  REQUIRE_THROWS_AS(Read(&tokenizer), SyntaxError);
  // return obj;
}

TEST_F(SchemeTest, Quote) {
  SECTION("Quote") {
    ReadFull("(1 . 2 3)");
    // std::cout << typeid(list).name() << '\n';
    // PrintTo(list, &std::cout);
  }
  ExpectEq("-323", "-323");
  ExpectEq("(quote (1 2))", "(1 2)");
  ExpectEq("'(1 2)", "(1 2)");
}
