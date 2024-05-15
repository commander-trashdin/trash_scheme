#include "specialform.h"
#include "gc.h"
#include "interfaces.h"
#include "util.h"
#include <algorithm>

SpecialForm::SpecialForm(const std::string &&name, ApplyMethod &&apply_method,
                         std::optional<size_t> arg_min,
                         std::optional<size_t> arg_max)
    : name_(std::move(name)), apply_method_(std::move(apply_method)) {}

GCTracked *SpecialForm::Apply(std::shared_ptr<Scope> &scope, GCTracked *args) {
  std::vector<GCTracked *> result;
  auto first = args;
  if (first->ID() != Types::cell)
    throw SyntaxError("Expected a list of arguments!");
  result.insert(result.end(), first->As<Cell>()->listbegin(),
                first->As<Cell>()->listend());
  if ((arg_min_ && result.size() < *arg_min_) ||
      (arg_max_ && result.size() > *arg_max_))
    throw SyntaxError("Wrong number of arguments!");
  return apply_method_(scope, result);
}

void SpecialForm::PrintTo(std::ostream *out) const {
  *out << "#<special form " << name_ << ">" << std::endl;
}