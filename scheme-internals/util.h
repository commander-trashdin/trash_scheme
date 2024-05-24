#include "interfaces.h"

static const std::string tSchemeExtension = ".trash";

[[nodiscard]]

bool hasCorrectExtension(const std::string &filename);

#define CHECKERR(result)                                                       \
  if (SubtypeOf(Types::error, result->ID()))                                   \
    return result;

class ControlTransfer : public std::exception {
public:
  [[nodiscard]] const char *what() const noexcept override { return "Exit"; }
};