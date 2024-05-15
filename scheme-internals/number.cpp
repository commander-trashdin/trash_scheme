#include "number.h"
#include "gc.h"

std::unordered_map<int64_t, GCTracked *> *Number::GetConstantRegistry() {
  return GCManager::GetInstance().GetNumReg();
}

int64_t Number::GetValue() const { return value_; }

bool Number::operator==(const Object &other) const {
  return other.ID() == ID() &&
         static_cast<Number *>(const_cast<Object *>(&other))->value_ == value_;
}