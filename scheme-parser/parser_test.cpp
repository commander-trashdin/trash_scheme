#include <catch.hpp>

#include <sstream>
#include <random>

#include <parser.h>

std::shared_ptr<Object> ReadFull(const std::string& str) {
    std::stringstream ss{str};
    Tokenizer tokenizer{&ss};

    auto obj = Read(&tokenizer);
    REQUIRE(tokenizer.IsEnd());
    return obj;
}

TEST_CASE("Read number") {
    auto node = ReadFull("5");
    REQUIRE(IsNumber(node));
    REQUIRE(AsNumber(node)->GetValue() == 5);

    node = ReadFull("-5");
    REQUIRE(IsNumber(node));
    REQUIRE(AsNumber(node)->GetValue() == -5);
}

std::string RandomSymbol(std::default_random_engine* rng) {
    std::uniform_int_distribution<char> symbol('a', 'z');
    std::string s;
    for (int i = 0; i < 5; ++i) {
        s.push_back(symbol(*rng));
    }
    return s;
}

TEST_CASE("Read symbol") {
    SECTION("Plus") {
        auto node = ReadFull("+");
        REQUIRE(IsSymbol(node));
        REQUIRE(AsSymbol(node)->GetName() == "+");
    }

    SECTION("Random symbol") {
        std::default_random_engine rng{42};
        for (int i = 0; i < 10; ++i) {
            auto name = RandomSymbol(&rng);
            auto node = ReadFull(name);
            REQUIRE(IsSymbol(node));
            REQUIRE(AsSymbol(node)->GetName() == name);
        }
    }
}

TEST_CASE("Lists") {
    SECTION("Empty list") {
        auto null = ReadFull("()");
        REQUIRE(!null);
    }

    SECTION("Pair") {
        auto pair = ReadFull("(1 . 2)");
        REQUIRE(IsCell(pair));

        auto first = AsCell(pair)->GetFirst();
        REQUIRE(IsNumber(first));
        REQUIRE(AsNumber(first)->GetValue() == 1);

        auto second = AsCell(pair)->GetSecond();
        REQUIRE(second->ID() == Types::numberType);
        REQUIRE(AsNumber(second)->GetValue() == 2);
    }

    SECTION("Simple list") {
        auto list = ReadFull("(1 2)");
        REQUIRE(IsCell(list));

        auto first = AsCell(list)->GetFirst();
        REQUIRE(IsNumber(first));
        REQUIRE(AsNumber(first)->GetValue() == 1);

        list = AsCell(list)->GetSecond();
        auto second = AsCell(list)->GetFirst();
        REQUIRE(IsNumber(second));
        REQUIRE(AsNumber(second)->GetValue() == 2);

        REQUIRE(!AsCell(list)->GetSecond());
    }

    SECTION("List with operator") {
        auto list = ReadFull("(+ 1 2)");
        REQUIRE(IsCell(list));

        auto first = AsCell(list)->GetFirst();
        REQUIRE(IsSymbol(first));
        REQUIRE(AsSymbol(first)->GetName() == "+");

        list = AsCell(list)->GetSecond();
        auto second = AsCell(list)->GetFirst();
        REQUIRE(IsNumber(second));
        REQUIRE(AsNumber(second)->GetValue() == 1);

        list = AsCell(list)->GetSecond();
        second = AsCell(list)->GetFirst();
        REQUIRE(IsNumber(second));
        REQUIRE(AsNumber(second)->GetValue() == 2);

        REQUIRE(!AsCell(list)->GetSecond());
    }

    SECTION("List with funny end") {
        auto list = ReadFull("(1 2 . 3)");

        REQUIRE(IsCell(list));

        auto first = AsCell(list)->GetFirst();
        REQUIRE(IsNumber(first));
        REQUIRE(AsNumber(first)->GetValue() == 1);

        list = AsCell(list)->GetSecond();
        auto second = AsCell(list)->GetFirst();
        REQUIRE(IsNumber(second));
        REQUIRE(AsNumber(second)->GetValue() == 2);

        auto last = AsCell(list)->GetSecond();
        REQUIRE(IsNumber(last));
        REQUIRE(AsNumber(last)->GetValue() == 3);
    }

    SECTION("Complex lists") {
        ReadFull("(1 . ())");
        ReadFull("(1 2 . ())");
        ReadFull("(1 . (2 . ()))");
        ReadFull("(1 2 (3 4) (()))");
        ReadFull("(+ 1 2 (- 3 4))");
    }

    SECTION("Invalid lists") {
        REQUIRE_THROWS_AS(ReadFull("("), SyntaxError);
        REQUIRE_THROWS_AS(ReadFull("(1"), SyntaxError);
        REQUIRE_THROWS_AS(ReadFull("(1 ."), SyntaxError);
        REQUIRE_THROWS_AS(ReadFull("( ."), SyntaxError);
        REQUIRE_THROWS_AS(ReadFull("(1 . ()"), SyntaxError);
        REQUIRE_THROWS_AS(ReadFull("(1 . )"), SyntaxError);
        REQUIRE_THROWS_AS(ReadFull("(1 . 2 3)"), SyntaxError);
    }
}
