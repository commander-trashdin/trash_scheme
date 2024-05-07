#pragma once

#include "../scheme-parser/parser.h"
#include <istream>
#include <memory>
#include <sstream>

class SchemeInterpreter {
public:
  SchemeInterpreter();

  ~SchemeInterpreter();

  Object *Eval(Object *in);

  void REPL(std::istream *in = &std::cin);

private:
  std::shared_ptr<Scope> global_scope_;
};
