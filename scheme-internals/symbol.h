#pragma once
#include "interfaces.h"

class Symbol : public Object {
public:
  using ValueType = std::string_view;

  explicit Symbol(std::string name);

  virtual Types ID() const override;

  virtual void PrintTo(std::ostream *out) const override;

  const std::string &GetName() const;

protected:
  const std::string name_;
};

class Boolean : public Symbol {
public:
  using ValueType = bool;

  explicit Boolean(bool val) : Symbol(val ? "#t" : "#f"), val_(val) {}

  bool IsFalse() const override;

private:
  const bool val_;
};
