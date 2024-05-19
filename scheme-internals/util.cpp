#include "util.h"
#include "cell.h"
#include "gc.h"

SyntaxError::SyntaxError(const std::string &what) : std::runtime_error(what) {}

NameError::NameError(const std::string &name)
    : std::runtime_error("variable not found: " + name) {}

RuntimeError::RuntimeError(const std::string &what)
    : std::runtime_error(what) {}
