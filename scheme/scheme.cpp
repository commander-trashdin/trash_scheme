#include "scheme.h"
#include "gc.h"
#include "parser.h"
#include <memory>

SchemeInterpreter::SchemeInterpreter() : global_scope_(Scope::Create()) {
  global_scope_->variables_["+"] = Object::Create<Function>("+", Plus);
  global_scope_->variables_["-"] = Object::Create<Function>("-", Minus);
  global_scope_->variables_["*"] = Object::Create<Function>("*", Multiply);
  global_scope_->variables_["/"] = Object::Create<Function>("/", Divide);
  global_scope_->variables_["if"] = Object::Create<SpecialForm>("if", If);
  global_scope_->variables_["quote"] =
      Object::Create<SpecialForm>("quote", Quote);
  global_scope_->variables_["null?"] =
      Object::Create<Function>("null?", CheckNull);
  global_scope_->variables_["pair?"] =
      Object::Create<Function>("pair?", CheckPair);
  global_scope_->variables_["number?"] =
      Object::Create<Function>("number?", CheckNumber);
  global_scope_->variables_["boolean?"] =
      Object::Create<Function>("boolean?", CheckBoolean);
  global_scope_->variables_["symbol?"] =
      Object::Create<Function>("symbol?", CheckSymbol);
  global_scope_->variables_["list?"] =
      Object::Create<Function>("list?", CheckList);
  global_scope_->variables_["eq?"] = Object::Create<Function>("eq?", Eq);
  global_scope_->variables_["integer-equal?"] =
      Object::Create<Function>("integer-equal?", IntegerEqual);
  global_scope_->variables_["not"] = Object::Create<Function>("not", Not);
  global_scope_->variables_["="] = Object::Create<Function>("=", Equality);
  global_scope_->variables_[">"] = Object::Create<Function>(">", More);
  global_scope_->variables_["<"] = Object::Create<Function>("<", Less);
  global_scope_->variables_[">="] = Object::Create<Function>(">=", MoreOrEqual);
  global_scope_->variables_["<="] = Object::Create<Function>("<=", LessOrEqual);
  global_scope_->variables_["min"] = Object::Create<Function>("min", Min);
  global_scope_->variables_["max"] = Object::Create<Function>("max", Max);
  global_scope_->variables_["abs"] = Object::Create<Function>("abs", Abs);
  global_scope_->variables_["cons"] = Object::Create<Function>("cons", Cons);
  global_scope_->variables_["car"] = Object::Create<Function>("car", Car);
  global_scope_->variables_["cdr"] = Object::Create<Function>("cdr", Cdr);
  global_scope_->variables_["set-car!"] =
      Object::Create<Function>("set-car!", SetCar);
  global_scope_->variables_["set-cdr!"] =
      Object::Create<Function>("set-cdr!", SetCdr);
  global_scope_->variables_["list"] = Object::Create<Function>("list", List);
  global_scope_->variables_["list-ref"] =
      Object::Create<Function>("list-ref", ListRef);
  global_scope_->variables_["list-tail"] =
      Object::Create<Function>("list-tail", ListTail);
  global_scope_->variables_["and"] = Object::Create<SpecialForm>("and", And);
  global_scope_->variables_["or"] = Object::Create<SpecialForm>("or", Or);
  global_scope_->variables_["lambda"] =
      Object::Create<SpecialForm>("lambda", Lambda);
  global_scope_->variables_["define"] =
      Object::Create<SpecialForm>("define", Define);
  global_scope_->variables_["set!"] = Object::Create<SpecialForm>("set!", Set);
  global_scope_->variables_["exit"] = Object::Create<Function>("exit", Exit);
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
