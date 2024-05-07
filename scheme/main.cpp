#include "gc.h"
#include "parser.h"
#include "scheme.h"
#include <fstream>
#include <iostream>
#include <memory>

int main() {
  SchemeInterpreter sch_int;
  sch_int.REPL();
  // std::ofstream debugFile;
  // debugFile.open("debug.txt", std::ios::app);
  // GCManager::GetInstance().PrintObjectsDebug(&debugFile);
  // debugFile.flush();
  return 0;
}
