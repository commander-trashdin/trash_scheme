#pragma once

#include "../scheme-parser/parser.h"
#include <memory>
#include <sstream>

class SchemeInterpreter {
public:
  SchemeInterpreter();

  ~SchemeInterpreter();

  Object *Eval(Object *in);

private:
  std::shared_ptr<Scope> global_scope_;
};
