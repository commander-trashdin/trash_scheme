#pragma once
#include "interfaces.h"

class Symbol : public Object {
public:
  using ValueType = std::string;

  static Symbol *AllocIn(T *storage);
  static std::unordered_map<int64_t, GCTracked *> *GetConstantRegistry();
  explicit Symbol(std::string name);

  [[nodiscard]] Types ID() const override;

  void PrintTo(std::ostream *out) const override;

  bool operator==(const Object &other) const override;

  [[nodiscard]] const std::string &GetName() const;

protected:
  const std::string name_;
};

class Boolean : public Symbol {
public:
  using ValueType = bool;

  static Boolean *AllocIn(T *storage);
  [[nodiscard]] virtual Types ID() const override;
  explicit Boolean(bool val);

  [[nodiscard]] bool IsFalse() const override;

  bool operator==(const Object &other) const override;

private:
  const bool val_;
};
