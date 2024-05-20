#include "util.h"

SyntaxError::SyntaxError(const std::string &what) : std::runtime_error(what) {}

NameError::NameError(const std::string &name)
    : std::runtime_error("variable not found: " + name) {}

RuntimeError::RuntimeError(const std::string &what)
    : std::runtime_error(what) {}

bool hasCorrectExtension(const std::string &filename) {
  return filename.size() >= tSchemeExtension.size() &&
         filename.compare(filename.size() - tSchemeExtension.size(),
                          tSchemeExtension.size(), tSchemeExtension) == 0;
}
