#pragma once
#include "scheme.h"
#include <gtest/gtest.h>

struct SchemeTest : public ::testing::Test {
  SchemeTest() {}

  void ExpectEq(std::string expression, std::string result) {
    std::stringstream ss{expression};
    Tokenizer tokenizer{&ss};
    auto obj = Read(&tokenizer);
    auto res = sch_int_.Eval(obj);
    std::ostringstream os;
    PrintTo(res, &os);
    EXPECT_EQ(os.str(), result);
  }

  void ExpectNoError(std::string expression) {
    std::stringstream ss{expression};
    Tokenizer tokenizer{&ss};
    auto obj = Read(&tokenizer);
    EXPECT_NO_THROW(sch_int_.Eval(obj));
  }

  void ExpectSyntaxError(std::string expression) {
    std::stringstream ss{expression};
    Tokenizer tokenizer{&ss};
    EXPECT_THROW(sch_int_.Eval(Read(&tokenizer)), SyntaxError);
  }

  void ExpectRuntimeError(std::string expression) {
    std::stringstream ss{expression};
    Tokenizer tokenizer{&ss};
    auto obj = Read(&tokenizer);
    EXPECT_THROW(sch_int_.Eval(obj), RuntimeError);
  }

  void ExpectNameError(std::string expression) {
    std::stringstream ss{expression};
    Tokenizer tokenizer{&ss};
    auto obj = Read(&tokenizer);
    EXPECT_THROW(sch_int_.Eval(obj), NameError);
  }

  SchemeInterpreter sch_int_ = SchemeInterpreter();
};
