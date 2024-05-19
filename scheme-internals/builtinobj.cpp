#include "storage.h"
#include "util.h"

BuiltInObject *BuiltInObject::AllocIn(T *storage) { return &(storage->bio_); }

Types BuiltInObject::ID() const { return Types::builtin; }

void BuiltInObject::PrintTo(std::ostream *) const {
  throw RuntimeError("Cannot print builtin object!");
}