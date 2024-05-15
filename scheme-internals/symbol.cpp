#include "symbol.h"
#include "interfaces.h"
#include "storage.h"

Symbol *Symbol::AllocIn(T *storage) { return &(storage->s_); }

Symbol::Symbol(std::string name) : name_(std::move(name)) {}

Types Symbol::ID() const { return Types::symbol; }

void Symbol::PrintTo(std::ostream *out) const { *out << name_; }

const std::string &Symbol::GetName() const { return name_; }

bool Symbol::operator==(const Object &other) const {
  return other.ID() == ID() &&
         static_cast<Symbol *>(const_cast<Object *>(&other))->name_ == name_;
}

Boolean *Boolean::AllocIn(T *storage) { return &(storage->b_); }

Boolean::Boolean(bool val) : Symbol(val ? "#t" : "#f"), val_(val) {}

Types Boolean::ID() const { return Types::boolean; }

bool Boolean::IsFalse() const { return !val_; }

bool Boolean::operator==(const Object &other) const {
  return other.ID() == ID() &&
         static_cast<Boolean *>(const_cast<Object *>(&other))->val_ == val_;
}