#pragma once
#include "interfaces.h"

class Number : public Object {
public:
  using ValueType = int64_t;

  static Number *AllocIn(T *storage);
  static std::unordered_map<int64_t, GCTracked *> *GetConstantRegistry();

  explicit Number(int64_t value = 0) : value_(value) {}

  virtual Types ID() const override;

  virtual void PrintTo(std::ostream *out) const override;

  virtual bool operator==(const Object &other) const override;

  int64_t GetValue() const;

private:
  const int64_t value_;
};
