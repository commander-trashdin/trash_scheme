#include "gc.h"
#include "parser.h"
#include "scheme.h"
#include <fstream>
#include <iostream>
#include <memory>

int main() {
  SchemeInterpreter sch_int;
  sch_int.REPL();
  return 0;
}
