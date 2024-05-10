#pragma once

#include "../scheme-tokenizer/tokenizer.h"
#include <concepts>
#include <cstdint>
#include <istream>
#include <memory>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <sys/stat.h>
#include <unordered_map>
#include <utility>
#include <vector>

class Parser {
public:
  explicit Parser(Tokenizer &&tok);

  GCTracked *ReadList();

  GCTracked *Read();

  GCTracked *ReadProper();

private:
  void ParenClose();
  void ParenOpen();
  Tokenizer tokenizer_;
  std::unordered_map<std::string_view, Symbol *> sym_table_;
  int64_t paren_count_ = 0;
};
