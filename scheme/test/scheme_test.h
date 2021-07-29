#pragma once

#include <catch.hpp>
#include "scheme.h"

struct SchemeTest {
    SchemeTest() {
    }

    void ExpectEq(std::string expression, std::string result) {
        std::stringstream ss{expression};
        Tokenizer tokenizer{&ss};
        auto obj = Read(&tokenizer);
        auto res = sch_int_.Eval(obj);
        std::ostringstream os;
        PrintTo(res, &os);
        REQUIRE(os.str() == result);
    }

    void ExpectNoError(std::string expression) {
        std::stringstream ss{expression};
        Tokenizer tokenizer{&ss};
        auto obj = Read(&tokenizer);
        REQUIRE_NOTHROW(sch_int_.Eval(obj));
    }

    void ExpectSyntaxError(std::string expression) {
        std::stringstream ss{expression};
        Tokenizer tokenizer{&ss};
        REQUIRE_THROWS_AS(sch_int_.Eval(Read(&tokenizer)), SyntaxError);
    }

    void ExpectRuntimeError(std::string expression) {
        std::stringstream ss{expression};
        Tokenizer tokenizer{&ss};
        auto obj = Read(&tokenizer);
        REQUIRE_THROWS_AS(sch_int_.Eval(obj), RuntimeError);
    }

    void ExpectNameError(std::string expression) {
        std::stringstream ss{expression};
        Tokenizer tokenizer{&ss};
        auto obj = Read(&tokenizer);
        REQUIRE_THROWS_AS(sch_int_.Eval(obj), NameError);
    }

    SchemeInterpretor sch_int_ = SchemeInterpretor();
};
