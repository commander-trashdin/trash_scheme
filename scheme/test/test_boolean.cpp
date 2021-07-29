#include <test/scheme_test.h>

TEST_CASE_METHOD(SchemeTest, "BooleansAreSelfEvaluating") {
    ExpectEq("#t", "#t");
    ExpectEq("#f", "#f");
}

TEST_CASE_METHOD(SchemeTest, "BooleanPredicate") {
    ExpectEq("(boolean? #t)", "#t");
    ExpectEq("(boolean? #f)", "#t");
    ExpectEq("(boolean? 1)", "#f");
    ExpectEq("(boolean? '())", "#f");
}

TEST_CASE_METHOD(SchemeTest, "NotFunction") {
    ExpectEq("(not #f)", "#t");
    ExpectEq("(not #t)", "#f");
    ExpectEq("(not 1)", "#f");
    ExpectEq("(not 0)", "#f");
    ExpectEq("(not '())", "#f");
}

TEST_CASE_METHOD(SchemeTest, "NotFunctionInvalidCall") {
    ExpectRuntimeError("(not)");
    ExpectRuntimeError("(not #t #t)");
}

TEST_CASE_METHOD(SchemeTest, "AndSyntax") {
    // (and <test>)
    // The <test> expressions are evaluated from left to right, and the value of the first
    // expression that evaluates to a false value is returned. Any remaining expressions are not
    // evaluated. If all the expressions evaluate to true values, the value of the last expression
    // is returned. If there are no expressions then #t is returned.

    ExpectEq("(and)", "#t");
    ExpectEq("(and (= 2 2) (> 2 1))", "#t");
    ExpectEq("(and (= 2 2) (< 2 1))", "#f");
    ExpectEq("(and 1 2 'c '(f g))", "(f g)");
}

TEST_CASE_METHOD(SchemeTest, "AndOptimizesArgumentEvaluation") {
    ExpectNoError("(define x 1)");
    ExpectNoError("(and #f (set! x 2))");
    ExpectEq("x", "1");
}

TEST_CASE_METHOD(SchemeTest, "OrSyntax") {
    // (or <test>)
    // The <test> expressions are evaluated from left to right, and the value of the first
    // expression that evaluates to a true value is returned. Any remaining expressions are not
    // evaluated. If all expressions evaluate to false values, the value of the last expression is
    // returned. If there are no expressions then #f is returned.

    ExpectEq("(or)", "#f");
    ExpectEq("(or (not (= 2 2)) (> 2 1))", "#t");
    ExpectEq("(or #f (< 2 1))", "#f");
    ExpectEq("(or #f 1)", "1");
}

TEST_CASE_METHOD(SchemeTest, "OrOptimizesArgumentEvaluation") {
    ExpectNoError("(define x 1)");
    ExpectNoError("(or #t (set! x 2))");
    ExpectEq("x", "1");
}
