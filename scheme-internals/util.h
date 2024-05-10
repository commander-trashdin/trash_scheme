#include "interfaces.h"

inline void PrintTo(const Object *obj, std::ostream *out) {
  if (!obj) {
    *out << "()";
    return;
  }
  obj->PrintTo(out);
}

std::vector<Object *> ToVector(const Object *head);

inline std::string Print(const Object *obj);

template <DerivedFromObject Derived> Derived *Is(Object *obj) {
  return dynamic_cast<Derived *>(obj);
}

struct SyntaxError : public std::runtime_error {
  explicit SyntaxError(const std::string &what);
};

struct RuntimeError : public std::runtime_error {
  explicit RuntimeError(const std::string &what);
};

struct NameError : public std::runtime_error {
  explicit NameError(const std::string &what);
};
