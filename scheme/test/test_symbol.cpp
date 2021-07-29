#include <test/scheme_test.h>

TEST_CASE_METHOD(SchemeTest, "SymbolsAreNotSelfEvaluating") {
    ExpectNameError("x");

    ExpectEq("'x", "x");
    ExpectEq("(quote x)", "x");
}

TEST_CASE_METHOD(SchemeTest, "SymbolPredicate") {
    ExpectEq("(symbol? 'x)", "#t");
    ExpectEq("(symbol? 1)", "#f");
}

TEST_CASE_METHOD(SchemeTest, "SymbolsAreUsedAsVariableNames") {
    ExpectNoError("(define x (+ 1 2))");
    ExpectEq("x", "3");

    ExpectNoError("(define x (+ 2 2))");
    ExpectEq("x", "4");
}

TEST_CASE_METHOD(SchemeTest, "DefineInvalidSyntax") {
    ExpectSyntaxError("(define)");
    ExpectSyntaxError("(define 1)");
    ExpectSyntaxError("(define x 1 2)");
}

TEST_CASE_METHOD(SchemeTest, "SetOverrideVariables") {
    ExpectNameError("(set! x 2)");
    ExpectNameError("x");

    ExpectNoError("(define x 1)");
    ExpectEq("x", "1");

    ExpectNoError("(set! x (+ 2 4))");
    ExpectEq("x", "6");
}

TEST_CASE_METHOD(SchemeTest, "SetInvalidSyntax") {
    ExpectSyntaxError("(set!)");
    ExpectSyntaxError("(set! 1)");
    ExpectSyntaxError("(set! x 1 2)");
}
