#include "parser.h"
#include "scheme.h"
#include <iostream>
#include <memory>

int main() {
  SchemeInterpreter sch_int;
  std::string expression;
  std::getline(std::cin, expression);
  while (true) {
    std::stringstream ss{expression};
    Tokenizer tokenizer{&ss};
    auto obj = sch_int.Eval(Read(&tokenizer));
    if (std::dynamic_pointer_cast<BuiltInObject>(obj) != nullptr)
      break;
    PrintTo(obj, &std::cout);
    std::cout << "\n";
    std::getline(std::cin, expression);
  }
  return 0;
}
