#pragma once
#include "interfaces.h"

class String : public Object {
public:
  using ValueType = std::string;

  static String *AllocIn(T *storage);
  explicit String(std::string val);

  [[nodiscard]] Types ID() const override;

  void PrintTo(std::ostream *out) const override;

  bool operator==(const Object &other) const override;

  [[nodiscard]] const std::string &GetValue() const;

private:
  const std::string val_;
};