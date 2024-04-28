#include "scheme_test.h"
#include <gtest/gtest.h>
#include <typeinfo>

void ReadFull(const std::string &str) {
  std::stringstream ss{str};
  Tokenizer tokenizer{&ss};

  EXPECT_THROW({ Read(&tokenizer); }, SyntaxError);
}

TEST_F(SchemeTest, QuoteHandling) {
  ReadFull("(1 . 2 3)");

  ExpectEq("-323", "-323");
  ExpectEq("(quote (1 2))", "(1 2)");
  ExpectEq("'(1 2)", "(1 2)");
}