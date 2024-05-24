#pragma once
#include "lisperrors.h"
#include "objects.h"

union T {
  Number n_;
  Symbol s_;
  Boolean b_;
  Function f_;
  LambdaFunction lf_;
  Cell c_;
  SpecialForm sf_;
  String str_;
  RuntimeError re_;
  SyntaxError se_;

  T() {}
  ~T() {}
};