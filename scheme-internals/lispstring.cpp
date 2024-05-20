#include "interfaces.h"
#include "storage.h"

String *String::AllocIn(T *storage) { return &(storage->str_); }

String::String(std::string val) : val_(std::move(val)) {}

[[nodiscard]] Types String::ID() const { return Types::string; }

void String::PrintTo(std::ostream *out) const { *out << "\"" << val_ << "\""; }

bool String::operator==(const Object &other) const {
  return other.ID() == ID() &&
         static_cast<String *>(const_cast<Object *>(&other))->val_ == val_;
}

[[nodiscard]] const std::string &String::GetValue() const { return val_; }
