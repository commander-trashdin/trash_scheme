#include "function.h"
#include "builtins.h"
#include "cell.h"
#include "gc.h"
#include "interfaces.h"
#include "symbol.h"
#include "util.h"
#include <algorithm>
#include <cstddef>
#include <vector>

std::optional<Types> Function::ArgType(size_t index) {
  if (auto type = std::get_if<Types>(&arg_types_); type != nullptr)
    return *type;
  auto v = std::get<std::vector<Types>>(arg_types_);
  if (index >= v.size())
    return std::nullopt;
  return v[index];
}

Function::Function(const std::string &&name,
                   std::variant<Types, std::vector<Types>> arg_types,
                   ApplyMethod &&apply_method)
    : name(name), arg_types_(arg_types), apply_method(apply_method) {}

GCTracked *Function::Apply(std::shared_ptr<Scope> &scope, GCTracked *args) {
  std::vector<GCTracked *> result;
  auto first = args;
  if (first->ID() != Types::cell)
    throw std::runtime_error("Expected a list of arguments!");

  auto it = first->As<Cell>()->listbegin();
  auto end = first->As<Cell>()->listend();
  size_t index = 0;
  while (it != end) {
    auto this_arg_type = ArgType(index);
    if (!this_arg_type)
      throw std::runtime_error("Too many arguments!");
    if (Is(*it, *this_arg_type))
      result.push_back(Eval(scope, {*it})); // How is this possible?
    else
      throw std::runtime_error("Wrong argument type!");
    ++it;
    ++index;
  }
  return apply_method(scope, result);
}

void Function::PrintTo(std::ostream *out) const {
  *out << "#<function " << name << ">" << std::endl;
}

LambdaFunction::LambdaFunction(std::shared_ptr<Scope> scope,
                               std::vector<GCTracked *> &&args,
                               std::span<GCTracked *const> body)
    : current_scope_(scope), args_(args), body_(body.begin(), body.end()) {}

LambdaFunction *LambdaFunction::AllocIn(T *storage) { return &(storage->lf_); }

GCTracked *LambdaFunction::Apply(std::shared_ptr<Scope> &scope,
                                 GCTracked *args) {
  if (args->ID() != Types::cell)
    throw std::runtime_error("Expected a list of arguments!");
  auto it = args->As<Cell>()->listbegin();
  size_t index = 0;
  while (it != args->As<Cell>()->listend()) {
    if (index >= args_.size())
      throw std::runtime_error("Too many arguments!");
    std::vector<GCTracked *> v = {*it};
    (*current_scope_)[args_[index]] = Eval(scope, v);
    ++it;
    ++index;
  }

  for (size_t ind = 0; ind < body_.size(); ++ind) {
    std::vector<GCTracked *> v = {body_[ind]};
    if (ind != body_.size() - 1)
      Eval(current_scope_, v);
    else
      return Eval(current_scope_, v);
  }
  return nullptr;
}

void LambdaFunction::PrintTo(std::ostream *out) const {
  *out << "#<lambda function>" << std::endl;
}

std::shared_ptr<Scope> LambdaFunction::GetScope() { return current_scope_; }

void LambdaFunction::SetScope(const std::shared_ptr<Scope> &scope) {
  current_scope_ = scope;
}

std::vector<GCTracked *> LambdaFunction::GetArgs() { return args_; }

void LambdaFunction::SetArgs(std::vector<GCTracked *> args) { args_ = args; }

std::vector<GCTracked *> LambdaFunction::GetBody() { return body_; }

void LambdaFunction::AddToBody(GCTracked *form) { body_.push_back(form); }