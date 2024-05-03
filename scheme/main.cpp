#include "gc.h"
#include "parser.h"
#include "scheme.h"
#include <fstream>
#include <iostream>
#include <memory>

int main() {
  SchemeInterpreter sch_int;
  std::string expression;
  std::getline(std::cin, expression);
  while (true) {
    std::stringstream ss{expression};
    Tokenizer tokenizer{&ss};
    auto obj = Read(&tokenizer);
    obj->Mark();
    auto res = sch_int.Eval(obj);
    obj->Unmark();
    if (dynamic_cast<BuiltInObject *>(res) != nullptr)
      break;
    PrintTo(res, &std::cout);
    GCManager::GetInstance().CollectGarbage();
    std::cout << "\n";
    std::getline(std::cin, expression);
  }
  // std::ofstream debugFile;
  // debugFile.open("debug.txt", std::ios::app);
  // GCManager::GetInstance().PrintObjectsDebug(&debugFile);
  // debugFile.flush();
  return 0;
}
