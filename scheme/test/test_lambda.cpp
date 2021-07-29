#include <test/scheme_test.h>

TEST_CASE_METHOD(SchemeTest, "SimpleLambda") {
    ExpectEq("((lambda (x) (+ 1 x)) 5)", "6");
}

TEST_CASE_METHOD(SchemeTest, "LambdaBodyHasImplicitBegin") {
    ExpectNoError("(define test (lambda (x) (set! x (* x 2)) (+ 1 x)))");
    ExpectEq("(test 20)", "41");
}

TEST_CASE_METHOD(SchemeTest, "SlowSum") {
    ExpectNoError("(define slow-add (lambda (x y) (if (= x 0) y (slow-add (- x 1) (+ y 1)))))");
    ExpectEq("(slow-add 3 3)", "6");
    ExpectEq("(slow-add 100 100)", "200");
}

TEST_CASE_METHOD(SchemeTest, "LambdaClosure") {
    ExpectNoError(R"EOF((define x 1))EOF");
    // R"EOF( is not part of the string. It is syntax for raw string literal in C++.
    // https://en.cppreference.com/w/cpp/language/string_literal
    ExpectNoError(R"EOF(
            (define range
              (lambda (x)
                (lambda ()
                  (set! x (+ x 1))
                  x)))
                        )EOF");

    ExpectNoError("(define my-range (range 10))");
    ExpectEq("(my-range)", "11");
    ExpectEq("(my-range)", "12");
    ExpectEq("(my-range)", "13");

    ExpectEq("x", "1");
}

TEST_CASE_METHOD(SchemeTest, "LambdaSyntax") {
    ExpectSyntaxError("(lambda)");
    ExpectSyntaxError("(lambda x)");
    ExpectSyntaxError("(lambda (x))");
}

TEST_CASE_METHOD(SchemeTest, "DefineLambdaSugar") {
    ExpectNoError("(define (inc x) (+ x 1))");
    ExpectEq("(inc -1)", "0");

    ExpectNoError("(define (add x y) (+ x y 1))");
    ExpectEq("(add -10 10)", "1");

    ExpectNoError("(define (zero) 0)");
    ExpectEq("(zero)", "0");
}
