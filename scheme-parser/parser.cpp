#include "parser.h"
#include "create.h"
#include "gc.h"
#include <cstdint>
#include <iostream>
#include <memory>
#include <span>
#include <string_view>
#include <utility>
#include <vector>

Scope::~Scope() {
  GCManager::GetInstance().RemoveRoot(this);
  for (auto &[_, obj] : variables_)
    obj->Unmark();
}

std::shared_ptr<Scope> Scope::Create() {
  std::shared_ptr<Scope> newScope = std::make_shared<Scope>();
  GCManager::GetInstance().AddRoot(newScope);
  return newScope;
}

std::shared_ptr<Scope> Scope::Create(std::shared_ptr<Scope> &parent) {
  std::shared_ptr<Scope> newScope = std::make_shared<Scope>(parent);
  GCManager::GetInstance().AddRoot(newScope);
  return newScope;
}

Object *Scope::Lookup(const std::string &name) {
  auto it = variables_.find(name);
  if (it == variables_.end()) {
    if (!parent_)
      throw NameError(name);
    else
      return parent_->Lookup(name);
  }
  return it->second;
}

Object *&Scope::operator[](Symbol *symbol) {
  return variables_[symbol->GetName()];
}

Object::~Object() {}

void Object::Mark(GCMark mark) {
  if (marked_ < mark) {
    marked_ = mark;
    MarkRelated(mark);
  }
}

void Object::Unmark(GCMark mark) {
  if (!(mark < marked_)) {
    marked_ = GCMark::White;
  }
}

void Object::MarkRelated(GCMark mark) {}
void Object::UnmarkRelated(GCMark mark) {}

bool Object::isMarked() const { return marked_ != GCMark::White; }

Types Object::ID() const { return Types::tType; }

bool Object::IsFalse() const { return false; }

Types BuiltInObject::ID() const { throw RuntimeError("Not a builtin type!"); }

void BuiltInObject::PrintTo(std::ostream *) const {
  throw RuntimeError("Cannot print builtin object!");
}
void BuiltInObject::PrintDebug(std::ostream *out) const {
  *out << "#<builtin>" << std::endl;
}
Object *BuiltInObject::Eval(std::shared_ptr<Scope> &) {
  throw RuntimeError("Cannot eval builtin object!");
}

template <typename... Sizes>
void SpecialForm::CheckArgs(const std::vector<Object *> &args, Kind kind,
                            Sizes... sizes) {
  std::vector<size_t> allowedSizes = {static_cast<unsigned long>(sizes)...};
  if (kind == Kind::Allow && std::find(allowedSizes.begin(), allowedSizes.end(),
                                       args.size()) == allowedSizes.end()) {
    throw SyntaxError("Wrong number of arguments!");
  }
  if (kind == Kind::Disallow &&
      std::find(allowedSizes.begin(), allowedSizes.end(), args.size()) !=
          allowedSizes.end()) {
    throw SyntaxError("Wrong number of arguments!");
  }
  return;
}

template <typename... Sizes>
void Function::CheckArgs(const std::vector<Object *> &args, Kind kind,
                         Sizes... sizes) {
  std::vector<size_t> allowedSizes = {static_cast<unsigned long>(sizes)...};
  if (kind == Kind::Allow && std::find(allowedSizes.begin(), allowedSizes.end(),
                                       args.size()) == allowedSizes.end()) {
    throw RuntimeError("Wrong number of arguments!");
  }
  if (kind == Kind::Disallow &&
      std::find(allowedSizes.begin(), allowedSizes.end(), args.size()) !=
          allowedSizes.end()) {
    throw RuntimeError("Wrong number of arguments!");
  }
  return;
}

SpecialForm::SpecialForm(const std::string &&name, ApplyMethod &&apply_method)
    : name(name), apply_method(apply_method) {}

Function::Function(const std::string &&name, ApplyMethod &&apply_method)
    : name(name), apply_method(apply_method) {}

Object *SpecialForm::Apply(std::shared_ptr<Scope> &scope,
                           const std::vector<Object *> &args) {
  return (this->apply_method)(scope, args);
}

Object *Function::Apply(std::shared_ptr<Scope> &,
                        const std::vector<Object *> &args) {
  return (this->apply_method)(args);
}

Cell::Cell() : head_(nullptr), tail_(nullptr) {}

Cell::Cell(Object *head, Object *tail) : head_(head), tail_(tail) {}

void Cell::MarkRelated(GCMark mark) {
  if (head_)
    head_->Mark(mark);
  if (tail_)
    tail_->Mark(mark);
}

void Cell::UnmarkRelated(GCMark mark) {
  if (head_)
    head_->Unmark(mark);
  if (tail_)
    tail_->Unmark(mark);
}

Types Cell::ID() const { return Types::cellType; }

void Cell::PrintTo(std::ostream *out) const {
  *out << '(';
  ::PrintTo(head_, out);
  auto next = AsCell(tail_);
  if (!next && tail_) {
    *out << " . ";
    ::PrintTo(tail_, out);
  } else {
    while (next) {
      *out << " ";
      ::PrintTo(next->head_, out);
      auto next_obj = AsCell(next->tail_);
      if (!next_obj && next->tail_) {
        *out << " . ";
        ::PrintTo(next->tail_, out);
      }
      next = next_obj;
    }
  }
  *out << ')';
  return;
}

void Cell::PrintDebug(std::ostream *out) const {
  PrintTo(out);
  *out << std::endl;
}

Object *Cell::Eval(std::shared_ptr<Scope> &scope) {
  auto lock = GCManager::SafeLock(this);
  if (head_ == nullptr)
    throw RuntimeError("First element of the list is not a function");

  auto ptr = head_->Eval(scope);
  auto fn = AsFunction(ptr);
  auto sf = dynamic_cast<SpecialForm *>(ptr);
  if (!fn && !sf)
    throw RuntimeError("First element of the list must be a function");

  std::vector<Object *> args = ToVector(tail_);
  if (fn)
    for (auto &arg : args) {
      arg = arg->Eval(scope);
      lock.Lock(arg);
    }

  return fn ? fn->Apply(scope, args) : sf->Apply(scope, args);
}

Object *Cell::GetFirst() const { return head_; }

void Cell::SetFirst(Object *object) { head_ = object; }

Object *Cell::GetSecond() const { return tail_; }

void Cell::SetSecond(Object *object) { tail_ = object; }

Number::Number() : value_(0) {}

Number::Number(int64_t value) : value_(value) {}

Types Number::ID() const { return Types::numberType; }

void Number::PrintTo(std::ostream *out) const { *out << value_; }
void Number::PrintDebug(std::ostream *out) const {
  PrintTo(out);
  *out << std::endl;
}

Object *Number::Eval(std::shared_ptr<Scope> &) { return this; }

int64_t Number::GetValue() const { return value_; }

int64_t &Number::SetValue() { return value_; }

std::unordered_map<int64_t, Number *> *Number::GetConstantRegistry() {
  return GCManager::GetInstance().GetNumReg();
}

Symbol::Symbol() : name_("") {}

Symbol::Symbol(std::string name) : name_(name) {}

Types Symbol::ID() const { return Types::symbolType; }

void Symbol::PrintTo(std::ostream *out) const { *out << name_; }
void Symbol::PrintDebug(std::ostream *out) const {
  PrintTo(out);
  *out << std::endl;
}

Object *Symbol::Eval(std::shared_ptr<Scope> &scope) {
  return scope->Lookup(name_);
}

const std::string &Symbol::GetName() const { return name_; }

std::string &Symbol::SetName() { return name_; }

std::unordered_map<std::string_view, Symbol *> *Symbol::GetConstantRegistry() {
  return GCManager::GetInstance().GetSymReg();
}

Object *Quote(std::shared_ptr<Scope> &, const std::vector<Object *> &args) {
  SpecialForm::CheckArgs(args, Kind::Allow, 1);
  return args[0];
}

Boolean::Boolean() {}

Boolean::Boolean(bool val) { name_ = val ? "#t" : "#f"; }

Object *Boolean::Eval(std::shared_ptr<Scope> &) { return this; }

bool Boolean::IsFalse() const { return this->GetName() == "#f"; }

void Function::PrintTo(std::ostream *) const {
  throw RuntimeError("can't print function");
}

void Function::PrintDebug(std::ostream *out) const {
  *out << "#<function " << name << ">" << std::endl;
}

void SpecialForm::PrintTo(std::ostream *) const {
  throw RuntimeError("can't print special form");
}

void SpecialForm::PrintDebug(std::ostream *out) const {
  *out << "#<special form " << name << ">" << std::endl;
}

Object *Function::Eval(std::shared_ptr<Scope> &) {
  throw RuntimeError("can't eval function");
}

Object *SpecialForm::Eval(std::shared_ptr<Scope> &) {
  throw RuntimeError("can't eval function");
}

Object *Plus(const std::vector<Object *> &args) {
  int64_t value = 0;
  for (const auto &arg : args) {
    auto number = dynamic_cast<Number *>(arg);
    if (!number)
      throw RuntimeError("+ arguments must be numbers");

    value += number->GetValue();
  }
  return Create<Number>(value);
}

Object *Minus(const std::vector<Object *> &args) {
  Function::CheckArgs(args, Kind::Disallow, 0);
  int64_t value = dynamic_cast<Number *>(args[0])->GetValue();
  if (!value)
    throw RuntimeError("- arguments must be numbers");

  for (size_t i = 1; i < args.size(); ++i) {
    auto number = dynamic_cast<Number *>(args[i]);
    if (!number)
      throw RuntimeError("- arguments must be numbers");
    value -= number->GetValue();
  }
  return Create<Number>(value);
}

Object *Divide(const std::vector<Object *> &args) {
  Function::CheckArgs(args, Kind::Disallow, 0);
  int64_t value = dynamic_cast<Number *>(args[0])->GetValue();
  if (!value)
    throw RuntimeError("/ arguments must be numbers");

  for (size_t i = 1; i < args.size(); ++i) {
    auto number = dynamic_cast<Number *>(args[i]);
    if (!number)
      throw RuntimeError("/ arguments must be numbers");
    value /= number->GetValue();
  }
  return Create<Number>(value);
}

Object *Multiply(const std::vector<Object *> &args) {
  int64_t value = 1;
  for (const auto &arg : args) {
    auto number = dynamic_cast<Number *>(arg);
    if (!number)
      throw RuntimeError("* arguments must be numbers");
    value *= number->GetValue();
  }
  return Create<Number>(value);
}

Object *If(std::shared_ptr<Scope> &scope, const std::vector<Object *> &args) {
  SpecialForm::CheckArgs(args, Kind::Allow, 2, 3);

  auto result = args[0]->Eval(scope);
  if (result && !result->IsFalse())
    return args[1]->Eval(scope);
  else
    return args.size() == 2 ? nullptr : args[2]->Eval(scope);
}

Object *CheckNull(const std::vector<Object *> &args) {
  Function::CheckArgs(args, Kind::Allow, 1);
  return Create<Boolean>(args[0] == nullptr);
}

Object *CheckPair(const std::vector<Object *> &args) {
  Function::CheckArgs(args, Kind::Allow, 1);
  return Create<Boolean>(IsCell(args[0]));
}

Object *CheckNumber(const std::vector<Object *> &args) {
  Function::CheckArgs(args, Kind::Allow, 1);
  return Create<Boolean>(IsNumber(args[0]));
}

Object *CheckBoolean(const std::vector<Object *> &args) {
  Function::CheckArgs(args, Kind::Allow, 1);
  return Create<Boolean>(IsSymbol(args[0]) &&
                         (AsSymbol(args[0])->GetName() == "#f" ||
                          AsSymbol(args[0])->GetName() == "#t"));
}

Object *CheckSymbol(const std::vector<Object *> &args) {
  Function::CheckArgs(args, Kind::Allow, 1);
  return Create<Boolean>(IsSymbol(args[0]));
}

Object *CheckList(const std::vector<Object *> &args) {
  Function::CheckArgs(args, Kind::Allow, 1);
  for (auto checker = args[0]; checker;
       checker = AsCell(checker)->GetSecond()) {
    if (!IsCell(checker))
      return Create<Boolean>(false);
  }
  return Create<Boolean>(true);
}

// FIXME
Object *Eq(const std::vector<Object *> &args) {
  Function::CheckArgs(args, Kind::Disallow, 0);
  return Create<Boolean>(args[0] == nullptr);
}

// FIXME
Object *Equal(const std::vector<Object *> &args) {
  Function::CheckArgs(args, Kind::Disallow, 0);
  return Create<Boolean>(args[0] == nullptr);
}

Object *IntegerEqual(const std::vector<Object *> &args) {
  if (args.size() == 0)
    return Create<Boolean>(true);

  if (!IsNumber(args[0]))
    throw RuntimeError("Syntax error!");

  int64_t value = AsNumber(args[0])->GetValue();
  for (const auto &obj : args) {
    if (!IsNumber(obj))
      throw RuntimeError("Syntax error!");

    if (value != AsNumber(obj)->GetValue())
      return Create<Boolean>(false);
  }
  return Create<Boolean>(true);
}

Object *Not(const std::vector<Object *> &args) {
  Function::CheckArgs(args, Kind::Allow, 1);

  return Create<Boolean>(args[0] == nullptr ? false : args[0]->IsFalse());
}

Object *Equality(const std::vector<Object *> &args) {
  if (args.size() == 0)
    return Create<Boolean>(true);

  if (!IsNumber(args[0]))
    throw RuntimeError("Syntax error!");

  int64_t value = AsNumber(args[0])->GetValue();
  for (const auto &obj : args) {
    if (!IsNumber(obj))
      throw RuntimeError("Syntax error!");

    if (value != AsNumber(obj)->GetValue())
      return Create<Boolean>(false);
  }
  return Create<Boolean>(true);
}

Object *More(const std::vector<Object *> &args) {
  for (size_t i = 1; i < args.size(); ++i) {
    if (!IsNumber(args[i]) || !IsNumber(args[i - 1]))
      throw RuntimeError("Syntax error!");

    if (AsNumber(args[i - 1])->GetValue() <= AsNumber(args[i])->GetValue())
      return Create<Boolean>(false);
  }
  return Create<Boolean>(true);
}

Object *Less(const std::vector<Object *> &args) {
  for (size_t i = 1; i < args.size(); ++i) {
    if (!IsNumber(args[i]) || !IsNumber(args[i - 1]))
      throw RuntimeError("Syntax error!");

    if (AsNumber(args[i - 1])->GetValue() >= AsNumber(args[i])->GetValue())
      return Create<Boolean>(false);
  }
  return Create<Boolean>(true);
}

Object *MoreOrEqual(const std::vector<Object *> &args) {
  for (size_t i = 1; i < args.size(); ++i) {
    if (!IsNumber(args[i]) || !IsNumber(args[i - 1]))
      throw RuntimeError("Syntax error!");

    if (AsNumber(args[i - 1])->GetValue() < AsNumber(args[i])->GetValue())
      return Create<Boolean>(false);
  }
  return Create<Boolean>(true);
}

Object *LessOrEqual(const std::vector<Object *> &args) {
  for (size_t i = 1; i < args.size(); ++i) {
    if (!IsNumber(args[i]) || !IsNumber(args[i - 1]))
      throw RuntimeError("Syntax error!");

    if (AsNumber(args[i - 1])->GetValue() > AsNumber(args[i])->GetValue())
      return Create<Boolean>(false);
  }
  return Create<Boolean>(true);
}

Object *Min(const std::vector<Object *> &args) {
  Function::CheckArgs(args, Kind::Disallow, 0);

  if (!IsNumber(args[0]))
    throw RuntimeError("Syntax error!");

  int64_t value = AsNumber(args[0])->GetValue();
  for (const auto &obj : args) {
    if (!IsNumber(obj))
      throw RuntimeError("Syntax error!");

    int64_t current = AsNumber(obj)->GetValue();
    if (value > current)
      value = current;
  }
  return Create<Number>(value);
}

Object *Max(const std::vector<Object *> &args) {
  Function::CheckArgs(args, Kind::Disallow, 0);

  if (!IsNumber(args[0]))
    throw RuntimeError("Syntax error!");

  int64_t value = AsNumber(args[0])->GetValue();
  for (const auto &obj : args) {
    if (!IsNumber(obj))
      throw RuntimeError("Syntax error!");

    int64_t current = AsNumber(obj)->GetValue();
    if (value < current)
      value = current;
  }
  return Create<Number>(value);
}

Object *Abs(const std::vector<Object *> &args) {
  Function::CheckArgs(args, Kind::Allow, 1);

  if (!IsNumber(args[0]))
    throw RuntimeError("Syntax error!");

  return Create<Number>(std::abs(AsNumber(args[0])->GetValue()));
}

Object *Cons(const std::vector<Object *> &args) {
  Function::CheckArgs(args, Kind::Allow, 2);

  return Create<Cell>(args[0], args[1]);
}

Object *Car(const std::vector<Object *> &args) {
  Function::CheckArgs(args, Kind::Allow, 1);

  if (!IsCell(args[0]))
    throw RuntimeError("Syntax error!");

  return AsCell(args[0])->GetFirst();
}

Object *Cdr(const std::vector<Object *> &args) {
  Function::CheckArgs(args, Kind::Allow, 1);

  if (!IsCell(args[0]))
    throw RuntimeError("Syntax error!");

  return AsCell(args[0])->GetSecond();
}

Object *SetCar(const std::vector<Object *> &args) {
  Function::CheckArgs(args, Kind::Allow, 2);

  if (!IsCell(args[0]))
    throw RuntimeError("Syntax error!");

  AsCell(args[0])->SetFirst(args[1]);
  return args[0];
}

Object *SetCdr(const std::vector<Object *> &args) {
  Function::CheckArgs(args, Kind::Allow, 2);

  if (!IsCell(args[0]))
    throw RuntimeError("Syntax error!");

  AsCell(args[0])->SetSecond(args[1]);
  return args[0];
}

Object *List(const std::vector<Object *> &args) {
  if (args.size() == 0)
    return nullptr;

  auto new_cell = Create<Cell>();
  auto res = new_cell;
  auto lock = GCManager::SafeLock(res);
  for (size_t ind = 0; ind < args.size();
       ++ind, new_cell = AsCell(new_cell->GetSecond())) {
    new_cell->SetFirst(args[ind]);
    if (ind != args.size() - 1)
      new_cell->SetSecond(Create<Cell>());
  }
  return res;
}

Object *ListRef(const std::vector<Object *> &args) {
  Function::CheckArgs(args, Kind::Allow, 2);

  if (!IsCell(args[0]) && !IsNumber(args[1]))
    throw RuntimeError("Arguments must be list and a number");

  auto scope = args[0];
  auto value = AsNumber(args[1])->GetValue();
  while (value != 0) {
    if (!IsCell(scope))
      throw RuntimeError("List is too short");
    scope = AsCell(scope)->GetSecond();
    --value;
  }
  if (!IsCell(scope))
    throw RuntimeError("List is too short");

  return AsCell(scope)->GetFirst();
}

Object *ListTail(const std::vector<Object *> &args) {
  Function::CheckArgs(args, Kind::Allow, 2);

  if (!IsCell(args[0]) && !IsNumber(args[1]))
    throw RuntimeError("Syntax error!");

  auto scope = args[0];
  auto value = AsNumber(args[1])->GetValue();
  while (value != 0) {
    if (!IsCell(scope))
      throw RuntimeError("Syntax error!");
    scope = AsCell(scope)->GetSecond();
    --value;
  }
  if (!IsCell(scope) && scope)
    throw RuntimeError("Syntax error!");
  else if (!scope)
    return nullptr;

  return AsCell(scope);
}

Object *Map(const std::vector<Object *> &args) {
  Function::CheckArgs(args, Kind::Allow, 2);

  if (!IsCell(args[1]) || !IsFunction(args[0]))
    throw RuntimeError("Syntax error!");

  auto new_cell = Create<Cell>();
  auto res = new_cell;
  auto lock = GCManager::SafeLock(res);
  auto source = AsCell(args[1]);
  auto fn = AsFunction(args[0]);
  while (true) {
    if (source->GetSecond() && !IsCell(source->GetSecond()))
      throw RuntimeError("Syntax error!");
    std::shared_ptr<Scope> nullscope = nullptr;
    auto result = fn->Apply(nullscope, {source->GetFirst()});
    new_cell->SetFirst(result);
    if (source->GetSecond()) {
      source = AsCell(source->GetSecond());
      new_cell->SetSecond(Create<Cell>());
      new_cell = AsCell(new_cell->GetSecond());
    } else {
      break;
    }
  }
  return res;
}

Object *And(std::shared_ptr<Scope> &scope, const std::vector<Object *> &args) {
  for (size_t ind = 0; ind < args.size(); ++ind) {
    auto res = args[ind]->Eval(scope);
    if (res->IsFalse())
      return Create<Boolean>(false);
    if (ind == args.size() - 1)
      return res;
  }
  return Create<Boolean>(true);
}

Object *Or(std::shared_ptr<Scope> &scope, const std::vector<Object *> &args) {
  for (size_t ind = 0; ind < args.size(); ++ind) {
    auto res = args[ind]->Eval(scope);
    if (!res->IsFalse())
      return res;
  }
  return Create<Boolean>(false);
}

Object *Define(std::shared_ptr<Scope> &scope,
               const std::vector<Object *> &args) {
  SpecialForm::CheckArgs(args, Kind::Allow, 2);

  if (IsSymbol(args[0])) {
    (*scope)[AsSymbol(args[0])] = args[1]->Eval(scope);
  } else if (IsCell(args[0])) {
    auto lambda_args = ToVector(AsCell(args[0])->GetSecond());
    for (const auto &arg : lambda_args)
      if (!IsSymbol(arg))
        throw SyntaxError("wrong argument name");

    (*scope)[AsSymbol(AsCell(args[0])->GetFirst())] = Create<LambdaFunction>(
        scope, std::move(lambda_args),
        std::span<Object *const>(args.begin() + 1, args.end()));
  }
  return nullptr;
}

Object *Set(std::shared_ptr<Scope> &scope, const std::vector<Object *> &args) {
  SpecialForm::CheckArgs(args, Kind::Allow, 2);

  if (IsSymbol(args[0])) {
    scope->Lookup(
        AsSymbol(args[0])->GetName()); // For the sake of error checking
    (*scope)[AsSymbol(args[0])] = args[1]->Eval(scope);
  } else {
    throw RuntimeError("Trying to set something that is not a variable");
  }
  return nullptr;
}

Object *Lambda(std::shared_ptr<Scope> &scope,
               const std::vector<Object *> &args) {
  SpecialForm::CheckArgs(args, Kind::Disallow, 1, 0);

  if (!IsCell(args[0]) && args[0])
    throw SyntaxError("Bad argument list!");
  auto lambda_args = ToVector(args[0]);
  for (const auto &arg : lambda_args)
    if (!IsSymbol(arg))
      throw SyntaxError("wrong argument name");

  auto fn = Create<LambdaFunction>(
      Scope::Create(scope), std::move(lambda_args),
      std::span<Object *const>(args.begin() + 1, args.end()));

  return fn;
}

void LambdaFunction::MarkRelated(GCMark mark) {
  for (auto &arg : args_)
    arg->Mark(mark);
  for (auto &body : body_)
    body->Mark(mark);
}

void LambdaFunction::UnmarkRelated(GCMark mark) {
  for (auto &arg : args_)
    arg->Unmark(mark);
  for (auto &body : body_)
    body->Unmark(mark);
}

Object *LambdaFunction::Apply(std::shared_ptr<Scope> &scope,
                              const std::vector<Object *> &args) {
  CheckArgs(args, Kind::Allow, args_.size());

  for (size_t ind = 0; ind < args.size(); ++ind)
    (*current_scope_)[AsSymbol(args_[ind])] = args[ind]->Eval(scope);

  for (size_t ind = 0; ind < body_.size(); ++ind) {
    if (ind != body_.size() - 1)
      body_[ind]->Eval(current_scope_);
    else
      return body_[ind]->Eval(current_scope_);
  }
  return nullptr;
}

Object *Exit(const std::vector<Object *> &args) {
  Function::CheckArgs(args, Kind::Allow, 0);
  return Create<BuiltInObject>();
}

SyntaxError::SyntaxError(const std::string &what) : std::runtime_error(what) {}

NameError::NameError(const std::string &name)
    : std::runtime_error("variable not found: " + name) {}

RuntimeError::RuntimeError(const std::string &what)
    : std::runtime_error(what) {}

bool IsNumber(const Object *obj) {
  return obj && Types::numberType == obj->ID();
}

Number *AsNumber(const Object *obj) {
  return IsNumber(obj) ? static_cast<Number *>(const_cast<Object *>(obj))
                       : nullptr;
}

bool IsCell(const Object *obj) { return obj && Types::cellType == obj->ID(); }

Cell *AsCell(const Object *obj) {
  return IsCell(obj) ? static_cast<Cell *>(const_cast<Object *>(obj)) : nullptr;
}

bool IsSymbol(const Object *obj) {
  return obj && Types::symbolType == obj->ID();
}

Symbol *AsSymbol(const Object *obj) {
  return IsSymbol(obj) ? static_cast<Symbol *>(const_cast<Object *>(obj))
                       : nullptr;
}

bool IsFunction(const Object *obj) {
  return obj && dynamic_cast<const Function *>(obj) != nullptr;
}

Function *AsFunction(const Object *obj) {
  return IsFunction(obj) ? static_cast<Function *>(const_cast<Object *>(obj))
                         : nullptr;
}

Object *ReadProper(Tokenizer *tokenizer) {
  if (tokenizer->IsEnd())
    return nullptr;

  auto current_object = tokenizer->GetToken();
  if (SymbolToken *symbol = std::get_if<SymbolToken>(&current_object)) {
    tokenizer->Next();
    if (symbol->name == "#t")
      return Create<Boolean>(true);
    else if (symbol->name == "#f")
      return Create<Boolean>(false);
    return Create<Symbol>(symbol->name);
  } else if (ConstantToken *constant_tok =
                 std::get_if<ConstantToken>(&current_object)) {
    tokenizer->Next();
    return Create<Number, constant>(static_cast<int64_t>(constant_tok->value));
  } else if (std::holds_alternative<QuoteToken>(current_object)) {
    tokenizer->Next();
    auto new_cell = Create<Cell>();
    auto list = Create<Cell>();
    new_cell->SetFirst(Create<Symbol>("quote"));
    list->SetFirst(ReadProper(tokenizer));
    new_cell->SetSecond(list);
    return new_cell;
  } else if (std::holds_alternative<DotToken>(current_object)) {
    throw SyntaxError("Unexpected symbol");
  } else if (BracketToken *bracket =
                 std::get_if<BracketToken>(&current_object)) {
    if (*bracket == BracketToken::CLOSE)
      throw SyntaxError("Unexpected closing parentheses");

    tokenizer->Next();
    return ReadList(tokenizer);
  }
  throw SyntaxError("Unexpected symbol");
}

Object *ReadList(Tokenizer *tokenizer) {
  if (tokenizer->IsEnd())
    throw SyntaxError("Input not complete");

  Object *head = nullptr;
  Cell *tail = nullptr;
  while (!tokenizer->IsEnd()) {
    auto current_token = tokenizer->GetToken();
    if (std::holds_alternative<BracketToken>(current_token) &&
        std::get<BracketToken>(current_token) == BracketToken::CLOSE) {
      tokenizer->Next();
      return head;
    } else if (std::holds_alternative<DotToken>(current_token)) {
      tokenizer->Next();
      if (tail == nullptr)
        throw SyntaxError("Improper list syntax");

      tail->SetSecond(ReadProper(tokenizer));
      if (!std::holds_alternative<BracketToken>(tokenizer->GetToken()) ||
          std::get<BracketToken>(tokenizer->GetToken()) != BracketToken::CLOSE)
        throw SyntaxError("Improper list syntax");

    } else {
      auto current_object = ReadProper(tokenizer);
      auto new_cell = Create<Cell>();
      new_cell->SetFirst(current_object);
      if (head == nullptr)
        head = new_cell;
      else
        tail->SetSecond(new_cell);
      tail = new_cell;
    }
  }
  throw SyntaxError("Unmatched opening parentheses");
}

Object *Read(Tokenizer *tokenizer) {
  auto obj = ReadProper(tokenizer);
  if (!tokenizer->IsEnd())
    throw SyntaxError("ImproperSyntax");

  return obj;
}
