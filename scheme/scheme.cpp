#include "scheme.h"
#include "create.h"
#include "gc.h"
#include "parser.h"
#include <memory>

SchemeInterpreter::SchemeInterpreter() : global_scope_(Scope::Create()) {
  global_scope_->variables_["+"] = Create<Function>("+", Plus);
  global_scope_->variables_["-"] = Create<Function>("-", Minus);
  global_scope_->variables_["*"] = Create<Function>("*", Multiply);
  global_scope_->variables_["/"] = Create<Function>("/", Divide);
  global_scope_->variables_["if"] = Create<SpecialForm>("if", If);
  global_scope_->variables_["quote"] = Create<SpecialForm>("quote", Quote);
  global_scope_->variables_["null?"] = Create<Function>("null?", CheckNull);
  global_scope_->variables_["pair?"] = Create<Function>("pair?", CheckPair);
  global_scope_->variables_["number?"] =
      Create<Function>("number?", CheckNumber);
  global_scope_->variables_["boolean?"] =
      Create<Function>("boolean?", CheckBoolean);
  global_scope_->variables_["symbol?"] =
      Create<Function>("symbol?", CheckSymbol);
  global_scope_->variables_["list?"] = Create<Function>("list?", CheckList);
  global_scope_->variables_["eq?"] = Create<Function>("eq?", Eq);
  global_scope_->variables_["integer-equal?"] =
      Create<Function>("integer-equal?", IntegerEqual);
  global_scope_->variables_["not"] = Create<Function>("not", Not);
  global_scope_->variables_["="] = Create<Function>("=", Equality);
  global_scope_->variables_[">"] = Create<Function>(">", More);
  global_scope_->variables_["<"] = Create<Function>("<", Less);
  global_scope_->variables_[">="] = Create<Function>(">=", MoreOrEqual);
  global_scope_->variables_["<="] = Create<Function>("<=", LessOrEqual);
  global_scope_->variables_["min"] = Create<Function>("min", Min);
  global_scope_->variables_["max"] = Create<Function>("max", Max);
  global_scope_->variables_["abs"] = Create<Function>("abs", Abs);
  global_scope_->variables_["cons"] = Create<Function>("cons", Cons);
  global_scope_->variables_["car"] = Create<Function>("car", Car);
  global_scope_->variables_["cdr"] = Create<Function>("cdr", Cdr);
  global_scope_->variables_["set-car!"] = Create<Function>("set-car!", SetCar);
  global_scope_->variables_["set-cdr!"] = Create<Function>("set-cdr!", SetCdr);
  global_scope_->variables_["list"] = Create<Function>("list", List);
  global_scope_->variables_["list-ref"] = Create<Function>("list-ref", ListRef);
  global_scope_->variables_["list-tail"] =
      Create<Function>("list-tail", ListTail);
  global_scope_->variables_["and"] = Create<SpecialForm>("and", And);
  global_scope_->variables_["or"] = Create<SpecialForm>("or", Or);
  global_scope_->variables_["lambda"] = Create<SpecialForm>("lambda", Lambda);
  global_scope_->variables_["define"] = Create<SpecialForm>("define", Define);
  global_scope_->variables_["set!"] = Create<SpecialForm>("set!", Set);
  global_scope_->variables_["exit"] = Create<Function>("exit", Exit);
  global_scope_->variables_["map"] = Create<Function>("map", Map);
}

SchemeInterpreter::~SchemeInterpreter() { global_scope_->variables_.clear(); }

Object *SchemeInterpreter::Eval(Object *in) {
  if (in == nullptr)
    throw RuntimeError("First element of the list must be function");

  return in->Eval(global_scope_);
}

inline std::string Print(const Object *obj) {
  std::stringstream ss;
  PrintTo(obj, &ss);
  return ss.str();
}

std::vector<Object *> ToVector(const Object *head) {
  std::vector<Object *> elements;
  if (!head) {
    return elements;
  } else {
    auto current = AsCell(head);
    for (auto current = AsCell(head); current;) {
      elements.push_back(current->GetFirst());
      auto next = current->GetSecond();
      if (!IsCell(next) && next)
        throw std::runtime_error("wrong argument list");

      current = AsCell(next);
    }
  }
  return elements;
}
