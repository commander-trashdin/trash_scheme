#include <catch.hpp>

#include <tokenizer.h>

#include <sstream>

TEST_CASE("Tokenizer works on simple case") {
    std::stringstream ss{"4+)'."};
    Tokenizer tokenizer{&ss};

    REQUIRE(!tokenizer.IsEnd());
    // Confused by compilation error? Think harder!
    REQUIRE(tokenizer.GetToken() == Token{ConstantToken{4}});

    tokenizer.Next();
    REQUIRE(!tokenizer.IsEnd());
    REQUIRE(tokenizer.GetToken() == Token{SymbolToken{"+"}});

    tokenizer.Next();
    REQUIRE(!tokenizer.IsEnd());
    REQUIRE(tokenizer.GetToken() == Token{BracketToken::CLOSE});

    tokenizer.Next();
    REQUIRE(!tokenizer.IsEnd());
    REQUIRE(tokenizer.GetToken() == Token{QuoteToken{}});

    tokenizer.Next();
    REQUIRE(!tokenizer.IsEnd());
    REQUIRE(tokenizer.GetToken() == Token{DotToken{}});

    tokenizer.Next();
    REQUIRE(tokenizer.IsEnd());
}

TEST_CASE("Negative numbers") {
    std::stringstream ss{"-2 - 2"};
    Tokenizer tokenizer{&ss};

    REQUIRE(!tokenizer.IsEnd());
    REQUIRE(tokenizer.GetToken() == Token{ConstantToken{-2}});

    tokenizer.Next();
    REQUIRE(!tokenizer.IsEnd());
    REQUIRE(tokenizer.GetToken() == Token{SymbolToken{"-"}});

    tokenizer.Next();
    REQUIRE(!tokenizer.IsEnd());
    REQUIRE(tokenizer.GetToken() == Token{ConstantToken{2}});
}

TEST_CASE("Symbol names") {
    std::stringstream ss{"foo bar zog-zog?"};
    Tokenizer tokenizer{&ss};

    REQUIRE(!tokenizer.IsEnd());
    REQUIRE(tokenizer.GetToken() == Token{SymbolToken{"foo"}});

    tokenizer.Next();
    REQUIRE(!tokenizer.IsEnd());
    REQUIRE(tokenizer.GetToken() == Token{SymbolToken{"bar"}});

    tokenizer.Next();
    REQUIRE(!tokenizer.IsEnd());
    REQUIRE(tokenizer.GetToken() == Token{SymbolToken{"zog-zog?"}});
}

TEST_CASE("GetToken is not moving") {
    std::stringstream ss{"1234+4"};
    Tokenizer tokenizer{&ss};

    REQUIRE(tokenizer.GetToken() == Token{ConstantToken{1234}});
    REQUIRE(tokenizer.GetToken() == Token{ConstantToken{1234}});
}

TEST_CASE("Tokenizer is streaming") {
    std::stringstream ss;
    ss << "2 ";

    Tokenizer tokenizer{&ss};
    REQUIRE(tokenizer.GetToken() == Token{ConstantToken{2}});

    ss << "* ";
    tokenizer.Next();
    REQUIRE(tokenizer.GetToken() == Token{SymbolToken{"*"}});

    ss << "2";
    tokenizer.Next();
    REQUIRE(tokenizer.GetToken() == Token{ConstantToken{2}});
}

TEST_CASE("Spaces are handled correctly") {
    SECTION("Just spaces") {
        std::stringstream ss{"      "};
        Tokenizer tokenizer{&ss};

        REQUIRE(tokenizer.IsEnd());
    }

    SECTION("Spaces between tokens") {
        std::stringstream ss{"  4 +  "};
        Tokenizer tokenizer{&ss};

        REQUIRE(!tokenizer.IsEnd());
        REQUIRE(tokenizer.GetToken() == Token{ConstantToken{4}});

        tokenizer.Next();
        REQUIRE(!tokenizer.IsEnd());
        REQUIRE(tokenizer.GetToken() == Token{SymbolToken{"+"}});

        tokenizer.Next();
        REQUIRE(tokenizer.IsEnd());
    }
}

TEST_CASE("Literal strings are handled correctly") {
    SECTION("Only EOFs and newlines") {
        std::string input = R"EOF(
                                   )EOF";
        std::stringstream ss{input};
        Tokenizer tokenizer{&ss};

        REQUIRE(tokenizer.IsEnd());
    }

    SECTION("String with tokens") {
        std::string input = R"EOF(
                            4 +
                            )EOF";
        std::stringstream ss{input};
        Tokenizer tokenizer{&ss};

        REQUIRE(!tokenizer.IsEnd());
        REQUIRE(tokenizer.GetToken() == Token{ConstantToken{4}});

        tokenizer.Next();
        REQUIRE(!tokenizer.IsEnd());
        REQUIRE(tokenizer.GetToken() == Token{SymbolToken{"+"}});

        tokenizer.Next();
        REQUIRE(tokenizer.IsEnd());
    }
}

TEST_CASE("Empty string handled correctly") {
    std::stringstream ss;
    Tokenizer tokenizer{&ss};

    REQUIRE(tokenizer.IsEnd());
}
