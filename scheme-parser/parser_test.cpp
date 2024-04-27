#include <gtest/gtest.h>
#include <random>
#include <sstream>

#include "parser.h"

std::shared_ptr<Object> ReadFull(const std::string &str) {
  std::stringstream ss{str};
  Tokenizer tokenizer{&ss};

  auto obj = Read(&tokenizer);
  EXPECT_TRUE(tokenizer.IsEnd());
  return obj;
}

TEST(ReadNumber, PositiveAndNegative) {
  auto node = ReadFull("5");
  EXPECT_TRUE(IsNumber(node));
  EXPECT_EQ(AsNumber(node)->GetValue(), 5);

  node = ReadFull("-5");
  EXPECT_TRUE(IsNumber(node));
  EXPECT_EQ(AsNumber(node)->GetValue(), -5);
}

std::string RandomSymbol(std::default_random_engine *rng) {
  std::uniform_int_distribution<char> symbol('a', 'z');
  std::string s;
  for (int i = 0; i < 5; ++i) {
    s.push_back(symbol(*rng));
  }
  return s;
}

TEST(ReadSymbol, Plus) {
  auto node = ReadFull("+");
  EXPECT_TRUE(IsSymbol(node));
  EXPECT_EQ(AsSymbol(node)->GetName(), "+");
}

TEST(ReadSymbol, RandomSymbol) {
  std::default_random_engine rng{42};
  for (int i = 0; i < 10; ++i) {
    auto name = RandomSymbol(&rng);
    auto node = ReadFull(name);
    EXPECT_TRUE(IsSymbol(node));
    EXPECT_EQ(AsSymbol(node)->GetName(), name);
  }
}

TEST(Lists, EmptyList) {
  auto null = ReadFull("()");
  EXPECT_FALSE(null);
}

TEST(Lists, Pair) {
  auto pair = ReadFull("(1 . 2)");
  EXPECT_TRUE(IsCell(pair));

  auto first = AsCell(pair)->GetFirst();
  EXPECT_TRUE(IsNumber(first));
  EXPECT_EQ(AsNumber(first)->GetValue(), 1);

  auto second = AsCell(pair)->GetSecond();
  EXPECT_EQ(second->ID(), Types::numberType);
  EXPECT_EQ(AsNumber(second)->GetValue(), 2);
}

TEST(Lists, SimpleList) {
  auto list = ReadFull("(1 2)");
  EXPECT_TRUE(IsCell(list));

  auto first = AsCell(list)->GetFirst();
  EXPECT_TRUE(IsNumber(first));
  EXPECT_EQ(AsNumber(first)->GetValue(), 1);

  list = AsCell(list)->GetSecond();
  auto second = AsCell(list)->GetFirst();
  EXPECT_TRUE(IsNumber(second));
  EXPECT_EQ(AsNumber(second)->GetValue(), 2);

  EXPECT_FALSE(AsCell(list)->GetSecond());
}

TEST(Lists, ListWithOperator) {
  auto list = ReadFull("(+ 1 2)");
  EXPECT_TRUE(IsCell(list));

  auto first = AsCell(list)->GetFirst();
  EXPECT_TRUE(IsSymbol(first));
  EXPECT_EQ(AsSymbol(first)->GetName(), "+");

  list = AsCell(list)->GetSecond();
  auto second = AsCell(list)->GetFirst();
  EXPECT_TRUE(IsNumber(second));
  EXPECT_EQ(AsNumber(second)->GetValue(), 1);

  list = AsCell(list)->GetSecond();
  second = AsCell(list)->GetFirst();
  EXPECT_TRUE(IsNumber(second));
  EXPECT_EQ(AsNumber(second)->GetValue(), 2);

  EXPECT_FALSE(AsCell(list)->GetSecond());
}

TEST(Lists, ListWithFunnyEnd) {
  auto list = ReadFull("(1 2 . 3)");

  EXPECT_TRUE(IsCell(list));

  auto first = AsCell(list)->GetFirst();
  EXPECT_TRUE(IsNumber(first));
  EXPECT_EQ(AsNumber(first)->GetValue(), 1);

  list = AsCell(list)->GetSecond();
  auto second = AsCell(list)->GetFirst();
  EXPECT_TRUE(IsNumber(second));
  EXPECT_EQ(AsNumber(second)->GetValue(), 2);

  auto last = AsCell(list)->GetSecond();
  EXPECT_TRUE(IsNumber(last));
  EXPECT_EQ(AsNumber(last)->GetValue(), 3);
}

TEST(Lists, ComplexLists) {
  ReadFull("(1 . ())");
  ReadFull("(1 2 . ())");
  ReadFull("(1 . (2 . ()))");
  ReadFull("(1 2 (3 4) (()))");
  ReadFull("(+ 1 2 (- 3 4))");
}

TEST(Lists, InvalidLists) {
  EXPECT_THROW(ReadFull("("), SyntaxError);
  EXPECT_THROW(ReadFull("(1"), SyntaxError);
  EXPECT_THROW(ReadFull("(1 ."), SyntaxError);
  EXPECT_THROW(ReadFull("( ."), SyntaxError);
  EXPECT_THROW(ReadFull("(1 . ()"), SyntaxError);
  EXPECT_THROW(ReadFull("(1 . )"), SyntaxError);
  EXPECT_THROW(ReadFull("(1 . 2 3)"), SyntaxError);
}
