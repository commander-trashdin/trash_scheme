#pragma once
#include "interfaces.h"

class Number : public Object {
public:
  using ValueType = int64_t;

  explicit Number(int64_t value = 0) : value_(value) {}

  virtual Types ID() const override;

  virtual void PrintTo(std::ostream *out) const override;

  int64_t GetValue() const { return value_; }

private:
  const int64_t value_;
};
