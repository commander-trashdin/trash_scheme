#include "gc.h"
#include "parser.h"
#include "scheme.h"
#include <fstream>
#include <iostream>
#include <memory>

int main(int argc, char *argv[]) {
  /*if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " <filename>\n";
    return 1;
  }*/

  std::string filename = argv[1];
  SchemeInterpreter sch_int;
  sch_int.Load(filename);

  return 0;
}
