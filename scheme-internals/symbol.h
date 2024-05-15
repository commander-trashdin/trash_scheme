#pragma once
#include "interfaces.h"

class Symbol : public Object {
public:
  using ValueType = std::string_view;

  static Symbol *AllocIn(T *storage);
  static std::unordered_map<int64_t, GCTracked *> *GetConstantRegistry();
  explicit Symbol(std::string name);

  virtual Types ID() const override;

  virtual void PrintTo(std::ostream *out) const override;

  virtual bool operator==(const Object &other) const override;

  const std::string &GetName() const;

protected:
  const std::string name_;
};

class Boolean : public Symbol {
public:
  using ValueType = bool;

  static Boolean *AllocIn(T *storage);
  virtual Types ID() const override;
  explicit Boolean(bool val);

  bool IsFalse() const override;

  virtual bool operator==(const Object &other) const override;

private:
  const bool val_;
};
