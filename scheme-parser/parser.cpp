#include "parser.h"
#include "gc.h"
#include "interfaces.h"
#include "tokenizer.h"
#include "util.h"
#include <variant>

Parser::Parser(Tokenizer &&tok) : tokenizer_(tok) {}

GCTracked *Parser::ParenClose() {
  paren_count_--;
  if (paren_count_ < 0)
    return Create<SyntaxError>("Unexpected closing parentheses!");

  return nullptr;
}
void Parser::ParenOpen() { paren_count_++; }

GCTracked *Parser::ReadProper() {
  if (tokenizer_.IsEnd())
    return nullptr;

  auto current_object = tokenizer_.GetToken().value();

  if (SymbolToken *symbol = std::get_if<SymbolToken>(&current_object)) {
    if (symbol->name == "#t")
      return Create<Boolean>(true);
    if (symbol->name == "#f")
      return Create<Boolean>(false);
    return Create<Symbol>(symbol->name);
  } else if (StringToken *str_tok = std::get_if<StringToken>(&current_object)) {
    return Create<String>(str_tok->name);
  } else if (NumberToken *num_tok = std::get_if<NumberToken>(&current_object)) {
    return Create<Number, constant>(num_tok->value);
  } else {
    auto syntax = std::get<SyntaxToken>(current_object);
    if (syntax == SyntaxToken::Quote) {
      auto new_cell = Create<Cell>();
      auto list = Create<Cell>();
      new_cell->As<Cell>()->SetFirst(Create<Symbol>("quote"));
      auto fst = ReadProper();
      if (SubtypeOf(Types::error, fst->ID()))
        return fst;
      list->As<Cell>()->SetFirst(fst);
      new_cell->As<Cell>()->SetSecond(list);
      return new_cell;
    } else if (syntax == SyntaxToken::Dot) {
      return Create<SyntaxError>("Unexpected symbol");
    } else {
      if (syntax == SyntaxToken::ParenClose) {
        auto error = ParenClose();
        if (error)
          return error;
      } else
        ParenOpen();
      return ReadList();
    }
  }
  return Create<SyntaxError>("Unexpected symbol");
}

GCTracked *Parser::ReadList() {
  tokenizer_.Next();
  if (tokenizer_.IsEnd())
    return Create<SyntaxError>("Input not complete");

  GCTracked *head = nullptr;
  GCTracked *tail = nullptr;
  while (true) {
    auto current_token = tokenizer_.GetToken().value();
    if (auto syntax = std::get_if<SyntaxToken>(&current_token);
        syntax != nullptr) {
      if (*syntax == SyntaxToken::ParenClose) {
        auto error = ParenClose();
        if (error)
          return error;
        if (paren_count_ != 0)
          tokenizer_.Next();
        return head;
      } else if (*syntax == SyntaxToken::Dot) {
        tokenizer_.Next();
        if (tail == nullptr)
          return Create<SyntaxError>("Improper list syntax");

        auto second = ReadProper();
        if (SubtypeOf(Types::error, second->ID()))
          return second;
        tail->As<Cell>()->SetSecond(second);
        tokenizer_.Next();
        if (auto syntax = std::get_if<SyntaxToken>(&current_token);
            !(syntax && *syntax == SyntaxToken::ParenClose))
          return Create<SyntaxError>("Improper list syntax");

        continue;
      }
    }
    auto current_object = ReadProper();
    if (SubtypeOf(Types::error, current_object->ID()))
      return current_object;
    if (current_object->ID() != Types::cell) // Kinda hacky
      tokenizer_.Next();
    auto new_cell = Create<Cell>();
    new_cell->As<Cell>()->SetFirst(current_object);
    if (head == nullptr)
      head = new_cell;
    else
      tail->As<Cell>()->SetSecond(new_cell);
    tail = new_cell;
  }
}

GCTracked *Parser::Read() {
  tokenizer_.Next();
  return ReadProper();
}