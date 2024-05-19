#include "scheme.h"
#include "builtins.h"
#include "gc.h"
#include "interfaces.h"
#include "parser.h"
#include "symbol.h"
#include "tokenizer.h"
#include <istream>
#include <memory>
#include <vector>

SchemeInterpreter::SchemeInterpreter() : global_scope_(Scope::Create()) {
  RegisterGlobalFn("+", Types::number, Plus);
  RegisterGlobalFn("-", Types::number, Minus);
  RegisterGlobalFn("*", Types::number, Multiply);
  RegisterGlobalFn("/", Types::number, Divide);
  RegisterSF("if", If, 2, 3);
  RegisterSF("quote", Quote, 1, 1);
  std::vector<Types> v = {Types::t};
  RegisterGlobalFn("null?", v, CheckNull);
  RegisterGlobalFn("pair?", v, CheckPair);
  RegisterGlobalFn("number?", v, CheckNumber);
  RegisterGlobalFn("boolean?", v, CheckBoolean);
  RegisterGlobalFn("symbol?", v, CheckSymbol);
  RegisterGlobalFn("list?", v, CheckList);
  v = {Types::t, Types::t};
  RegisterGlobalFn("eq?", v, Eq);
  RegisterGlobalFn("eql?", v, Eql);
  v = {Types::t};
  RegisterGlobalFn("not", v, Not);
  v = {Types::number, Types::number};
  RegisterGlobalFn(">", v, More);
  RegisterGlobalFn("<", v, Less);
  RegisterGlobalFn(">=", v, MoreOrEqual);
  RegisterGlobalFn("<=", v, LessOrEqual);
  RegisterGlobalFn("min", Types::number, Min);
  RegisterGlobalFn("max", Types::number, Max);
  v = {Types::t, Types::t};
  RegisterGlobalFn("cons", v, Cons);
  v = {Types::cell};
  RegisterGlobalFn("car", v, Car);
  RegisterGlobalFn("cdr", v, Cdr);
  v = {Types::cell, Types::t};
  RegisterGlobalFn("set-car!", v, SetCar);
  RegisterGlobalFn("set-cdr!", v, SetCdr);
  RegisterGlobalFn("list", Types::t, List);
  v = {Types::cell, Types::number};
  RegisterGlobalFn("list-ref", v, ListRef);
  RegisterGlobalFn("list-tail", Types::cell, ListRef);
  RegisterSF("and", And);
  RegisterSF("or", Or);
  RegisterSF("lambda", Lambda);
  RegisterSF("define", Define, 2, 2);
  RegisterSF("set!", Set, 2, 2);
  v = {Types::t};
  RegisterGlobalFn("eval", v, ::Eval);
  v = {Types::function, Types::cell};
  RegisterGlobalFn("map", v, Map);
  v.clear();
  RegisterGlobalFn("exit", v, Exit);
}

SchemeInterpreter::~SchemeInterpreter() { global_scope_->Clear(); }

void SchemeInterpreter::RegisterGlobalFn(
    const std::string name, std::variant<Types, std::vector<Types>> arg_types,
    GCTracked *(*fn)(std::shared_ptr<Scope> &,
                     const std::vector<GCTracked *> &)) {
  auto func = Create<Function>(name, arg_types, std::move(fn));
  (*global_scope_)[Create<Symbol, constant>(name)] = func;
}

void SchemeInterpreter::RegisterSF(
    const std::string name,
    GCTracked *(*sf)(std::shared_ptr<Scope> &,
                     const std::vector<GCTracked *> &),
    std::optional<size_t> arg_min, std::optional<size_t> arg_max) {
  auto sform = Create<SpecialForm>(name, std::move(sf), arg_min, arg_max);
  (*global_scope_)[Create<Symbol, constant>(name)] = sform;
}

GCTracked *SchemeInterpreter::Eval(GCTracked *in) {
  if (in == nullptr)
    throw std::runtime_error("First element of the list must be function");
  return ::Eval(global_scope_, {in});
}

inline std::string Print(const Object *obj) {
  std::stringstream ss;
  obj->PrintTo(&ss);
  return ss.str();
}

void SchemeInterpreter::REPL(std::istream *in) {
  Parser parser((Tokenizer(in)));
  while (true) {
    std::cout << "> ";
    std::cout.flush();
    GCManager::GetInstance().SetPhase(Phase::Read);
    auto obj = parser.Read();
    GCManager::GetInstance().SetPhase(Phase::Eval);
    auto res = Eval(obj);
    if (res->ID() == Types::builtin)
      break;
    res->PrintTo(&std::cout);

    std::cout << "\n";
  }
}