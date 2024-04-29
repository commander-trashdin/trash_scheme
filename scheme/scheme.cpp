#include "scheme.h"
#include "parser.h"

SchemeInterpreter::SchemeInterpreter()
    : global_scope_(std::make_shared<Scope>()) {
  global_scope_->variables_["+"] = std::make_shared<Plus>();
  global_scope_->variables_["-"] = std::make_shared<Minus>();
  global_scope_->variables_["*"] = std::make_shared<Multiply>();
  global_scope_->variables_["/"] = std::make_shared<Divide>();
  global_scope_->variables_["if"] = std::make_shared<If>();
  global_scope_->variables_["quote"] = std::make_shared<Quote>();
  global_scope_->variables_["null?"] = std::make_shared<CheckNull>();
  global_scope_->variables_["pair?"] = std::make_shared<CheckPair>();
  global_scope_->variables_["number?"] = std::make_shared<CheckNumber>();
  global_scope_->variables_["boolean?"] = std::make_shared<CheckBoolean>();
  global_scope_->variables_["symbol?"] = std::make_shared<CheckSymbol>();
  global_scope_->variables_["list?"] = std::make_shared<CheckList>();
  global_scope_->variables_["eq?"] = std::make_shared<Eq>();
  global_scope_->variables_["integer-equal?"] =
      std::make_shared<IntegerEqual>();
  global_scope_->variables_["not"] = std::make_shared<Not>();
  global_scope_->variables_["="] = std::make_shared<Equality>();
  global_scope_->variables_[">"] = std::make_shared<More>();
  global_scope_->variables_["<"] = std::make_shared<Less>();
  global_scope_->variables_[">="] = std::make_shared<MoreOrEqual>();
  global_scope_->variables_["<="] = std::make_shared<LessOrEqual>();
  global_scope_->variables_["min"] = std::make_shared<Min>();
  global_scope_->variables_["max"] = std::make_shared<Max>();
  global_scope_->variables_["abs"] = std::make_shared<Abs>();
  global_scope_->variables_["cons"] = std::make_shared<Cons>();
  global_scope_->variables_["car"] = std::make_shared<Car>();
  global_scope_->variables_["cdr"] = std::make_shared<Cdr>();
  global_scope_->variables_["set-car!"] = std::make_shared<SetCar>();
  global_scope_->variables_["set-cdr!"] = std::make_shared<SetCdr>();
  global_scope_->variables_["list"] = std::make_shared<List>();
  global_scope_->variables_["list-ref"] = std::make_shared<ListRef>();
  global_scope_->variables_["list-tail"] = std::make_shared<ListTail>();
  global_scope_->variables_["and"] = std::make_shared<And>();
  global_scope_->variables_["or"] = std::make_shared<Or>();
  global_scope_->variables_["lambda"] = std::make_shared<Lambda>();
  global_scope_->variables_["define"] = std::make_shared<Define>();
  global_scope_->variables_["set!"] = std::make_shared<Set>();
  global_scope_->variables_["exit"] = std::make_shared<Exit>();
}

SchemeInterpreter::~SchemeInterpreter() { global_scope_->variables_.clear(); }

std::shared_ptr<Object> SchemeInterpreter::Eval(std::shared_ptr<Object> in) {
  if (in == nullptr)
    throw RuntimeError("First element of the list must be function");

  return in->Eval(global_scope_);
}

inline std::string Print(const std::shared_ptr<Object> &obj) {
  std::stringstream ss;
  PrintTo(obj, &ss);
  return ss.str();
}

std::vector<std::shared_ptr<Object>>
ToVector(const std::shared_ptr<Object> &head) {
  std::vector<std::shared_ptr<Object>> elements;
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
