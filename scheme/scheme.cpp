#include "scheme.h"
#include "builtins.h"
#include "gc.h"
#include "interfaces.h"
#include "parser.h"
#include "symbol.h"
#include "tokenizer.h"
#include "util.h"
#include <fstream>
#include <istream>
#include <memory>
#include <vector>

SchemeInterpreter::SchemeInterpreter() : global_scope_(Scope::Create()) {
  RegisterSF("and", And);
  RegisterSF("or", Or);
  RegisterSF("lambda", Lambda);
  RegisterSF("define", Define, 2, 2);
  RegisterSF("set!", Set, 2, 2);
  RegisterSF("if", If, 2, 3);
  RegisterSF("quote", Quote, 1, 1);
  RegisterSF("print-debug", PrintDebug, 1, 1);
  RegisterGlobalFn("+", Types::number, Plus);
  RegisterGlobalFn("-", Types::number, Minus);
  RegisterGlobalFn("*", Types::number, Multiply);
  RegisterGlobalFn("/", Types::number, Divide);
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
  v = {Types::t};
  RegisterGlobalFn("eval", v, ::Eval);
  v = {Types::function, Types::cell};
  RegisterGlobalFn("map", v, Map);
  v = {Types::string};
  RegisterGlobalFn("load", v, ::Load);
  v = {Types::t};
  RegisterGlobalFn("print", v, Print);
  v.clear();
  RegisterGlobalFn("exit", v, Exit);
  RegisterGlobalFn("read", v, Read);
}

SchemeInterpreter::~SchemeInterpreter() { global_scope_->Clear(); }

void SchemeInterpreter::RegisterGlobalFn(
    std::string name, std::variant<Types, std::vector<Types>> arg_types,
    GCTracked *(*fn)(std::shared_ptr<Scope> &,
                     const std::vector<GCTracked *> &)) {
  auto func = Create<Function>(name, arg_types, std::move(fn));
  (*global_scope_)[Create<Symbol, constant>(name)] = func;
}

void SchemeInterpreter::RegisterSF(
    std::string name,
    GCTracked *(*sf)(std::shared_ptr<Scope> &,
                     const std::vector<GCTracked *> &),
    std::optional<size_t> arg_min, std::optional<size_t> arg_max) {
  auto sform = Create<SpecialForm>(name, std::move(sf), arg_min, arg_max);
  (*global_scope_)[Create<Symbol, constant>(name)] = sform;
}

GCTracked *SchemeInterpreter::Eval(GCTracked *in) {
  if (in == nullptr)
    throw std::runtime_error("Evalutation of null object");
  return ::Eval(global_scope_, {in});
}

inline std::string Print(const Object *obj) {
  std::stringstream ss;
  obj->PrintTo(&ss);
  return ss.str();
}

void SchemeInterpreter::Load(const std::string &filename) {
  if (!hasCorrectExtension(filename))
    throw std::runtime_error("Invalid file extension");
  std::ifstream filestream(filename);
  if (!filestream.is_open())
    throw std::runtime_error("Failed to open file");
  GCTracked *res = nullptr;
  while (true) {
    Parser parser((Tokenizer(&filestream)));
    GCManager::GetInstance().SetPhase(Phase::Read);
    auto obj = parser.Read();
    if (obj == nullptr)
      break;
    if (SubtypeOf(Types::error, obj->ID())) {
      obj->PrintTo(&std::cerr);
      break;
    }
    GCManager::GetInstance().SetPhase(Phase::Eval); // insert try catch here
    res = ::Eval(global_scope_, {obj});
    if (SubtypeOf(Types::error, res->ID())) {
      res->PrintTo(&std::cerr);
      break;
    }
  }
}