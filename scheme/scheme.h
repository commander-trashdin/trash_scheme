#pragma once

#include "../scheme-parser/parser.h"
#include <memory>
#include <sstream>

class SchemeInterpreter {
public:
  SchemeInterpreter();

  ~SchemeInterpreter();

  std::shared_ptr<Object> Eval(std::shared_ptr<Object> in);

private:
  std::shared_ptr<Scope> global_scope_;
};
