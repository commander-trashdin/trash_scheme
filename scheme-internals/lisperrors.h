#pragma once
#include "interfaces.h"

class LispError : public Object {
public:
  using ValueType = std::string;

  explicit LispError(std::string message);

  [[nodiscard]] Types ID() const override;

protected:
  const std::string message_;
};

class RuntimeError : public LispError {
public:
  static RuntimeError *AllocIn(T *storage);

  explicit RuntimeError(std::string message);

  [[nodiscard]] Types ID() const override;

  void PrintTo(std::ostream *out) const override;
};

class SyntaxError : public LispError {
public:
  static SyntaxError *AllocIn(T *storage);

  explicit SyntaxError(std::string message);

  [[nodiscard]] Types ID() const override;

  void PrintTo(std::ostream *out) const override;
};