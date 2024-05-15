#include "util.h"
#include "cell.h"
#include "gc.h"

bool Is(const GCTracked *obj, Types type) { return obj->ID() == type; }

SyntaxError::SyntaxError(const std::string &what) : std::runtime_error(what) {}

NameError::NameError(const std::string &name)
    : std::runtime_error("variable not found: " + name) {}

RuntimeError::RuntimeError(const std::string &what)
    : std::runtime_error(what) {}
