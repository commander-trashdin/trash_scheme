#include "number.h"
#include "gc.h"
#include "interfaces.h"
#include "storage.h"

Number *Number::AllocIn(T *storage) { return &(storage->n_); }

std::unordered_map<int64_t, GCTracked *> *Number::GetConstantRegistry() {
  return GCManager::GetInstance().GetNumReg();
}

Types Number::ID() const { return Types::number; }

void Number::PrintTo(std::ostream *out) const { *out << value_; }

int64_t Number::GetValue() const { return value_; }

bool Number::operator==(const Object &other) const {
  return other.ID() == ID() &&
         static_cast<Number *>(const_cast<Object *>(&other))->value_ == value_;
}
