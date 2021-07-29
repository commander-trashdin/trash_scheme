#include <test/scheme_test.h>

TEST_CASE_METHOD(SchemeTest, "IfReturnValue") {
    ExpectEq("(if #t 0)", "0");
    ExpectEq("(if #f 0)", "()");
    ExpectEq("(if (= 2 2) (+ 1 10))", "11");
    ExpectEq("(if (= 2 3) (+ 1 10) 5)", "5");
}

TEST_CASE_METHOD(SchemeTest, "IfEvaluation") {
    ExpectNoError("(define x 1)");

    ExpectNoError("(if #f (set! x 2))");
    ExpectEq("x", "1");

    ExpectNoError("(if #t (set! x 4) (set! x 3))");
    ExpectEq("x", "4");
}

TEST_CASE_METHOD(SchemeTest, "IfSyntax") {
    ExpectSyntaxError("(if)");
    ExpectSyntaxError("(if 1 2 3 4)");
}
