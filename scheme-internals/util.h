#include "interfaces.h"

static const std::string tSchemeExtension = ".trash";

[[nodiscard]]

bool hasCorrectExtension(const std::string &filename);

struct SyntaxError : public std::runtime_error {
  explicit SyntaxError(const std::string &what);
};

struct RuntimeError : public std::runtime_error {
  explicit RuntimeError(const std::string &what);
};

struct NameError : public std::runtime_error {
  explicit NameError(const std::string &what);
};
