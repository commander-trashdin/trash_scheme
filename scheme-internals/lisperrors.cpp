#include "lisperrors.h"
#include "storage.h"

LispError::LispError(std::string message) : message_(std::move(message)) {}

[[nodiscard]] Types LispError::ID() const { return Types::error; }

RuntimeError *RuntimeError::AllocIn(T *storage) { return &(storage->re_); }

RuntimeError::RuntimeError(std::string message)
    : LispError(std::move(message)) {}

[[nodiscard]] Types RuntimeError::ID() const { return Types::runtimeerror; }

void RuntimeError::PrintTo(std::ostream *out) const {
  *out << "Runtime error:\n " << message_;
}

SyntaxError *SyntaxError::AllocIn(T *storage) { return &(storage->se_); }

SyntaxError::SyntaxError(std::string message) : LispError(std::move(message)) {}

[[nodiscard]] Types SyntaxError::ID() const { return Types::syntaxerror; }

void SyntaxError::PrintTo(std::ostream *out) const {
  *out << "Syntax error:\n " << message_;
}