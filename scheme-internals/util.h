#include "interfaces.h"

inline std::string Print(const GCTracked *obj);

struct SyntaxError : public std::runtime_error {
  explicit SyntaxError(const std::string &what);
};

struct RuntimeError : public std::runtime_error {
  explicit RuntimeError(const std::string &what);
};

struct NameError : public std::runtime_error {
  explicit NameError(const std::string &what);
};
