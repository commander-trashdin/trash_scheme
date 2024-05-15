#include "parser.h"
#include "interfaces.h"
#include "objects.h"
#include "tokenizer.h"
#include "util.h"
#include <variant>

Parser::Parser(Tokenizer &&tok) : tokenizer_(tok) {}

void Parser::ParenClose() {
  paren_count_--;
  if (paren_count_ < 0)
    throw SyntaxError("Unexpected closing parentheses!");
}
void Parser::ParenOpen() { paren_count_++; }

GCTracked *Parser::ReadProper() {
  if (tokenizer_.IsEnd())
    return nullptr;

  auto current_object = tokenizer_.GetToken().value();

  if (SymbolToken *symbol = std::get_if<SymbolToken>(&current_object)) {
    tokenizer_.Next();
    if (symbol->name == "#t")
      return Create<Boolean>(true);
    else if (symbol->name == "#f")
      return Create<Boolean>(false);
    return Create<Symbol>(symbol->name);
  } else if (NumberToken *num_tok = std::get_if<NumberToken>(&current_object)) {
    tokenizer_.Next();
    return Create<Number, constant>(num_tok->value);
  } else {
    auto syntax = std::get<SyntaxToken>(current_object);
    if (syntax == SyntaxToken::Quote) {
      tokenizer_.Next();
      auto new_cell = Create<Cell>();
      auto list = Create<Cell>();
      new_cell->As<Cell>()->SetFirst(Create<Symbol>("quote"));
      list->As<Cell>()->SetFirst(ReadProper());
      new_cell->As<Cell>()->SetSecond(list);
      return new_cell;
    } else if (syntax == SyntaxToken::Dot) {
      throw SyntaxError("Unexpected symbol");
    } else {
      if (syntax == SyntaxToken::ParenClose)
        ParenClose();
      else
        ParenOpen();

      tokenizer_.Next();
      return ReadList();
    }
  }
  throw SyntaxError("Unexpected symbol");
}

GCTracked *Parser::ReadList() {
  if (tokenizer_.IsEnd())
    throw SyntaxError("Input not complete");

  GCTracked *head = nullptr;
  GCTracked *tail = nullptr;
  while (true) {
    auto current_token = tokenizer_.GetToken().value();
    if (auto syntax = std::get_if<SyntaxToken>(&current_token);
        syntax != nullptr) {
      if (*syntax == SyntaxToken::ParenClose) {
        ParenClose();
        if (paren_count_ != 0)
          tokenizer_.Next();
        return head;
      } else if (*syntax == SyntaxToken::Dot) {
        tokenizer_.Next();
        if (tail == nullptr)
          throw SyntaxError("Improper list syntax");

        tail->As<Cell>()->SetSecond(ReadProper());
        if (auto syntax = std::get_if<SyntaxToken>(&current_token);
            !(syntax && *syntax == SyntaxToken::ParenClose))
          throw SyntaxError("Improper list syntax");

        continue;
      }
    }
    auto current_object = ReadProper();
    auto new_cell = Create<Cell>();
    new_cell->As<Cell>()->SetFirst(current_object);
    if (head == nullptr)
      head = new_cell;
    else
      tail->As<Cell>()->SetSecond(new_cell);
    tail = new_cell;
  }
  throw SyntaxError("Unmatched opening parentheses");
}

GCTracked *Parser::Read() {
  tokenizer_.Next();
  return ReadProper();
}