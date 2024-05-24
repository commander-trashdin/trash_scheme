#include "builtins.h"
#include "cell.h"
#include "gc.h"
#include "interfaces.h"
#include "lisperrors.h"
#include "storage.h"
#include "util.h"
#include <cstddef>
#include <utility>
#include <variant>
#include <vector>

Function *Function::AllocIn(T *storage) { return &(storage->f_); }

std::optional<Types> Function::ArgType(size_t index) {
  if (auto type = std::get_if<Types>(&arg_types_); type != nullptr)
    return *type;
  auto v = std::get<std::vector<Types>>(arg_types_);
  if (index >= v.size())
    return std::nullopt;
  return v[index];
}

Function::Function(std::string name,
                   std::variant<Types, std::vector<Types>> arg_types,
                   ApplyMethod &&apply_method)
    : name(std::move(name)), arg_types_(std::move(arg_types)),
      apply_method(apply_method) {}

GCTracked *Function::Apply(std::shared_ptr<Scope> &scope, GCTracked *args) {
  auto lock = GCManager::GetInstance().Guard(args);
  std::vector<GCTracked *> result;
  if (args->ID() == Types::null) {
    if (ArgType(0) && std::holds_alternative<std::vector<Types>>(arg_types_))
      return Create<RuntimeError>("Too few arguments!");
    else
      return apply_method(scope, result);
  }
  auto first = args;
  if (first->ID() != Types::cell)
    return Create<RuntimeError>("Expected a list of arguments!");

  auto it = first->As<Cell>()->listbegin();
  auto end = first->As<Cell>()->listend();
  size_t index = 0;
  while (it != end) {
    auto this_arg_type = ArgType(index);
    if (!this_arg_type)
      return Create<RuntimeError>("Too many arguments!");
    auto arg = Eval(scope, {*it});
    lock.Lock(arg);
    CHECKERR(arg);
    if (SubtypeOf(*this_arg_type, arg->ID()))
      result.push_back(arg);
    else
      return Create<RuntimeError>("Wrong argument type!");
    ++it;
    ++index;
  }
  if (ArgType(index) && std::holds_alternative<std::vector<Types>>(arg_types_))
    return Create<RuntimeError>("Too few arguments!");

  return apply_method(scope, result);
}

Types Function::ID() const { return Types::function; }

void Function::PrintTo(std::ostream *out) const {
  *out << "#<function " << name << ">";
}

LambdaFunction *LambdaFunction::AllocIn(T *storage) { return &(storage->lf_); }

LambdaFunction::LambdaFunction(std::shared_ptr<Scope> scope,
                               std::vector<GCTracked *> &&args,
                               std::span<GCTracked *const> body)
    : current_scope_(std::move(scope)), args_(args),
      body_(body.begin(), body.end()) {}

GCTracked *LambdaFunction::Apply(std::shared_ptr<Scope> &scope,
                                 GCTracked *args) {
  auto lock = GCManager::GetInstance().Guard(args);
  if (args->ID() != Types::null) {
    auto it = args->As<Cell>()->listbegin();
    size_t index = 0;
    while (it != args->As<Cell>()->listend()) {
      if (index >= args_.size())
        return Create<RuntimeError>("Too many arguments!");
      std::vector<GCTracked *> v = {*it};
      auto to_add = Eval(scope, v);
      lock.Lock(to_add);
      CHECKERR(to_add);
      (*current_scope_)[args_[index]] = to_add;
      ++it;
      ++index;
    }
    if (index < args_.size())
      return Create<RuntimeError>("Too few arguments!");
  }

  for (size_t ind = 0; ind < body_.size(); ++ind) {
    std::vector<GCTracked *> v = {body_[ind]};
    if (ind != body_.size() - 1) {
      auto _res = Eval(current_scope_, v);
      lock.Lock(_res);
      CHECKERR(_res);
    } else
      return Eval(current_scope_, v);
  }
  return nullptr;
}

void LambdaFunction::PrintTo(std::ostream *out) const {
  *out << "#<lambda function>" << std::endl;
}

Types LambdaFunction::ID() const { return Types::function; }

std::shared_ptr<Scope> LambdaFunction::GetScope() { return current_scope_; }

void LambdaFunction::SetScope(const std::shared_ptr<Scope> &scope) {
  current_scope_ = scope;
}

std::vector<GCTracked *> LambdaFunction::GetArgs() { return args_; }

void LambdaFunction::SetArgs(std::vector<GCTracked *> args) { args_ = args; }

std::vector<GCTracked *> LambdaFunction::GetBody() { return body_; }

void LambdaFunction::AddToBody(GCTracked *form) { body_.push_back(form); }

void LambdaFunction::Walk(const std::function<void(GCTracked *)> &fn) {
  for (auto &[key, obj] : current_scope_->variables_) {
    fn(key);
    fn(obj);
  }
  for (auto obj : args_)
    fn(obj);
  for (auto obj : body_)
    fn(obj);
}