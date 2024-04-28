#include "tokenizer.h"
#include <gtest/gtest.h>

#include <sstream>

TEST(TokenizerTests, WorksOnSimpleCase) {
  std::stringstream ss{"4+)'."};
  Tokenizer tokenizer{&ss};

  EXPECT_FALSE(tokenizer.IsEnd());
  EXPECT_EQ(tokenizer.GetToken(), Token{ConstantToken{4}});

  tokenizer.Next();
  EXPECT_FALSE(tokenizer.IsEnd());
  EXPECT_EQ(tokenizer.GetToken(), Token{SymbolToken{"+"}});

  tokenizer.Next();
  EXPECT_FALSE(tokenizer.IsEnd());
  EXPECT_EQ(tokenizer.GetToken(), Token{BracketToken::CLOSE});

  tokenizer.Next();
  EXPECT_FALSE(tokenizer.IsEnd());
  EXPECT_EQ(tokenizer.GetToken(), Token{QuoteToken{}});

  tokenizer.Next();
  EXPECT_FALSE(tokenizer.IsEnd());
  EXPECT_EQ(tokenizer.GetToken(), Token{DotToken{}});

  tokenizer.Next();
  EXPECT_TRUE(tokenizer.IsEnd());
}

TEST(TokenizerTests, HandlesNegativeNumbers) {
  std::stringstream ss{"-2 - 2"};
  Tokenizer tokenizer{&ss};

  EXPECT_FALSE(tokenizer.IsEnd());
  EXPECT_EQ(tokenizer.GetToken(), Token{ConstantToken{-2}});

  tokenizer.Next();
  EXPECT_FALSE(tokenizer.IsEnd());
  EXPECT_EQ(tokenizer.GetToken(), Token{SymbolToken{"-"}});

  tokenizer.Next();
  EXPECT_FALSE(tokenizer.IsEnd());
  EXPECT_EQ(tokenizer.GetToken(), Token{ConstantToken{2}});
}

TEST(TokenizerTests, HandlesSymbolNames) {
  std::stringstream ss{"foo bar zog-zog?"};
  Tokenizer tokenizer{&ss};

  EXPECT_FALSE(tokenizer.IsEnd());
  EXPECT_EQ(tokenizer.GetToken(), Token{SymbolToken{"foo"}});

  tokenizer.Next();
  EXPECT_FALSE(tokenizer.IsEnd());
  EXPECT_EQ(tokenizer.GetToken(), Token{SymbolToken{"bar"}});

  tokenizer.Next();
  EXPECT_FALSE(tokenizer.IsEnd());
  EXPECT_EQ(tokenizer.GetToken(), Token{SymbolToken{"zog-zog?"}});
}

TEST(TokenizerTests, GetTokenIsNotMoving) {
  std::stringstream ss{"1234+4"};
  Tokenizer tokenizer{&ss};

  EXPECT_EQ(tokenizer.GetToken(), Token{ConstantToken{1234}});
  EXPECT_EQ(tokenizer.GetToken(), Token{ConstantToken{1234}});
}

TEST(TokenizerTests, IsStreaming) {
  std::stringstream ss;
  ss << "2 ";

  Tokenizer tokenizer{&ss};
  EXPECT_EQ(tokenizer.GetToken(), Token{ConstantToken{2}});

  ss << "* ";
  tokenizer.Next();
  EXPECT_EQ(tokenizer.GetToken(), Token{SymbolToken{"*"}});

  ss << "2";
  tokenizer.Next();
  EXPECT_EQ(tokenizer.GetToken(), Token{ConstantToken{2}});
}

TEST(TokenizerTests, HandlesSpacesCorrectly) {
  std::stringstream ss{"      "};
  Tokenizer tokenizer{&ss};

  EXPECT_TRUE(tokenizer.IsEnd());

  ss.str("  4 +  ");
  ss.clear();
  tokenizer = Tokenizer(&ss);

  EXPECT_FALSE(tokenizer.IsEnd());
  EXPECT_EQ(tokenizer.GetToken(), Token{ConstantToken{4}});

  tokenizer.Next();
  EXPECT_FALSE(tokenizer.IsEnd());
  EXPECT_EQ(tokenizer.GetToken(), Token{SymbolToken{"+"}});

  tokenizer.Next();
  EXPECT_TRUE(tokenizer.IsEnd());
}

TEST(TokenizerTests, HandlesLiteralStringsCorrectly) {
  std::string input = R"EOF(
                                   )EOF";
  std::stringstream ss{input};
  Tokenizer tokenizer{&ss};

  EXPECT_TRUE(tokenizer.IsEnd());

  input = R"EOF(
                            4 +
                            )EOF";
  ss.str(input);
  ss.clear();
  tokenizer = Tokenizer(&ss);

  EXPECT_FALSE(tokenizer.IsEnd());
  EXPECT_EQ(tokenizer.GetToken(), Token{ConstantToken{4}});

  tokenizer.Next();
  EXPECT_FALSE(tokenizer.IsEnd());
  EXPECT_EQ(tokenizer.GetToken(), Token{SymbolToken{"+"}});

  tokenizer.Next();
  EXPECT_TRUE(tokenizer.IsEnd());
}

TEST(TokenizerTests, HandlesEmptyStringCorrectly) {
  std::stringstream ss;
  Tokenizer tokenizer{&ss};

  EXPECT_TRUE(tokenizer.IsEnd());
}
