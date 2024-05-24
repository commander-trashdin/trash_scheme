#include "util.h"

bool hasCorrectExtension(const std::string &filename) {
  return filename.size() >= tSchemeExtension.size() &&
         filename.compare(filename.size() - tSchemeExtension.size(),
                          tSchemeExtension.size(), tSchemeExtension) == 0;
}
