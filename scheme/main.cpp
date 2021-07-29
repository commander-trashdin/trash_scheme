#include <iostream>
#include "scheme.h"

int main() {
    SchemeInterpretor sch_int;
    std::string expression;
    std::getline(std::cin, expression);
    while (expression != "(exit)") {
        std::stringstream ss{expression};
        Tokenizer tokenizer{&ss};
        PrintTo(sch_int.Eval(Read(&tokenizer)), &std::cout);
        std::cout << "\n";
        std::getline(std::cin, expression);
    }
    return 0;
}
