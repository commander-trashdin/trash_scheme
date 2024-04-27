#pragma once

#include "../scheme-parser/parser.h"
#include <functional>
#include <memory>
#include <sstream>
#include <string>

class SchemeInterpreter {
public:
  SchemeInterpreter();

  ~SchemeInterpreter();

  std::shared_ptr<Object> Eval(std::shared_ptr<Object> in);

private:
  std::shared_ptr<Scope> global_scope_;
};
