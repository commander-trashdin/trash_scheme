#include "parser.h"
#include <iostream>

Scope::~Scope() {
  GCManager::GetInstance().RemoveRoot(this);
  for (auto &[_, obj] : variables_)
    obj->Unmark();
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
Object::~Object() { GCManager::GetInstance().UnregisterObject(this); }

void Object::Mark() {
  marked_ = true;
  MarkRelated();
}

void Object::Unmark() {
  marked_ = false;
  UnmarkRelated();
}

void Object::MarkRelated() {}
void Object::UnmarkRelated() {}

bool Object::isMarked() const { return marked_; }

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

Object *Function::Apply(std::shared_ptr<Scope> &scope,
                        const std::vector<Object *> &args) {
  return (this->apply_method)(scope, args);
}

Cell::Cell() : head_(nullptr), tail_(nullptr) {}

Cell::Cell(Object *head, Object *tail) : head_(head), tail_(tail) {}

void Cell::MarkRelated() {
  if (head_)
    head_->Mark();
  if (tail_)
    tail_->Mark();
}

void Cell::UnmarkRelated() {
  if (head_)
    head_->Unmark();
  if (tail_)
    tail_->Unmark();
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
  if (head_ == nullptr)
    throw RuntimeError("First element of the list is not a function");

  auto ptr = head_->Eval(scope);
  auto fn = dynamic_cast<Function *>(ptr);
  auto sf = dynamic_cast<SpecialForm *>(ptr);
  if (!fn && !sf)
    throw RuntimeError("First element of the list must be a function");

  std::vector<Object *> args = ToVector(tail_);
  if (fn)
    for (auto &arg : args)
      arg = arg->Eval(scope);

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

Object *Quote(std::shared_ptr<Scope> &scope,
              const std::vector<Object *> &args) {
  std::ignore = scope;
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

Object *Plus(std::shared_ptr<Scope> &, const std::vector<Object *> &args) {
  int64_t value = 0;
  for (const auto &arg : args) {
    auto number = dynamic_cast<Number *>(arg);
    if (!number)
      throw RuntimeError("+ arguments must be numbers");

    value += number->GetValue();
  }
  return Object::Create<Number>(value);
}

Object *Minus(std::shared_ptr<Scope> &, const std::vector<Object *> &args) {
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
  return Object::Create<Number>(value);
}

Object *Divide(std::shared_ptr<Scope> &, const std::vector<Object *> &args) {
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
  return Object::Create<Number>(value);
}

Object *Multiply(std::shared_ptr<Scope> &, const std::vector<Object *> &args) {
  int64_t value = 1;
  for (const auto &arg : args) {
    auto number = dynamic_cast<Number *>(arg);
    if (!number)
      throw RuntimeError("* arguments must be numbers");
    value *= number->GetValue();
  }
  return Object::Create<Number>(value);
}

Object *If(std::shared_ptr<Scope> &scope, const std::vector<Object *> &args) {
  SpecialForm::CheckArgs(args, Kind::Allow, 2, 3);

  auto result = args[0]->Eval(scope);
  if (result && !result->IsFalse())
    return args[1]->Eval(scope);
  else
    return args.size() == 2 ? nullptr : args[2]->Eval(scope);
}

Object *CheckNull(std::shared_ptr<Scope> &, const std::vector<Object *> &args) {
  Function::CheckArgs(args, Kind::Allow, 1);
  return Object::Create<Boolean>(args[0] == nullptr);
}

Object *CheckPair(std::shared_ptr<Scope> &, const std::vector<Object *> &args) {
  Function::CheckArgs(args, Kind::Allow, 1);
  return Object::Create<Boolean>(IsCell(args[0]));
}

Object *CheckNumber(std::shared_ptr<Scope> &,
                    const std::vector<Object *> &args) {
  Function::CheckArgs(args, Kind::Allow, 1);
  return Object::Create<Boolean>(IsNumber(args[0]));
}

Object *CheckBoolean(std::shared_ptr<Scope> &,
                     const std::vector<Object *> &args) {
  Function::CheckArgs(args, Kind::Allow, 1);
  return Object::Create<Boolean>(IsSymbol(args[0]) &&
                                 (AsSymbol(args[0])->GetName() == "#f" ||
                                  AsSymbol(args[0])->GetName() == "#t"));
}

Object *CheckSymbol(std::shared_ptr<Scope> &,
                    const std::vector<Object *> &args) {
  Function::CheckArgs(args, Kind::Allow, 1);
  return Object::Create<Boolean>(IsSymbol(args[0]));
}

Object *CheckList(std::shared_ptr<Scope> &, const std::vector<Object *> &args) {
  Function::CheckArgs(args, Kind::Allow, 1);
  for (auto checker = args[0]; checker;
       checker = AsCell(checker)->GetSecond()) {
    if (!IsCell(checker))
      return Object::Create<Boolean>(false);
  }
  return Object::Create<Boolean>(true);
}

// FIXME
Object *Eq(std::shared_ptr<Scope> &, const std::vector<Object *> &args) {
  Function::CheckArgs(args, Kind::Disallow, 0);
  return Object::Create<Boolean>(args[0] == nullptr);
}

// FIXME
Object *Equal(std::shared_ptr<Scope> &, const std::vector<Object *> &args) {
  Function::CheckArgs(args, Kind::Disallow, 0);
  return Object::Create<Boolean>(args[0] == nullptr);
}

Object *IntegerEqual(std::shared_ptr<Scope> &,
                     const std::vector<Object *> &args) {
  if (args.size() == 0)
    return Object::Create<Boolean>(true);

  if (!IsNumber(args[0]))
    throw RuntimeError("Syntax error!");

  int64_t value = AsNumber(args[0])->GetValue();
  for (const auto &obj : args) {
    if (!IsNumber(obj))
      throw RuntimeError("Syntax error!");

    if (value != AsNumber(obj)->GetValue())
      return Object::Create<Boolean>(false);
  }
  return Object::Create<Boolean>(true);
}

Object *Not(std::shared_ptr<Scope> &, const std::vector<Object *> &args) {
  Function::CheckArgs(args, Kind::Allow, 1);

  return Object::Create<Boolean>(args[0] == nullptr ? false
                                                    : args[0]->IsFalse());
}

Object *Equality(std::shared_ptr<Scope> &, const std::vector<Object *> &args) {
  if (args.size() == 0)
    return Object::Create<Boolean>(true);

  if (!IsNumber(args[0]))
    throw RuntimeError("Syntax error!");

  int64_t value = AsNumber(args[0])->GetValue();
  for (const auto &obj : args) {
    if (!IsNumber(obj))
      throw RuntimeError("Syntax error!");

    if (value != AsNumber(obj)->GetValue())
      return Object::Create<Boolean>(false);
  }
  return Object::Create<Boolean>(true);
}

Object *More(std::shared_ptr<Scope> &, const std::vector<Object *> &args) {
  for (size_t i = 1; i < args.size(); ++i) {
    if (!IsNumber(args[i]) || !IsNumber(args[i - 1]))
      throw RuntimeError("Syntax error!");

    if (AsNumber(args[i - 1])->GetValue() <= AsNumber(args[i])->GetValue())
      return Object::Create<Boolean>(false);
  }
  return Object::Create<Boolean>(true);
}

Object *Less(std::shared_ptr<Scope> &, const std::vector<Object *> &args) {
  for (size_t i = 1; i < args.size(); ++i) {
    if (!IsNumber(args[i]) || !IsNumber(args[i - 1]))
      throw RuntimeError("Syntax error!");

    if (AsNumber(args[i - 1])->GetValue() >= AsNumber(args[i])->GetValue())
      return Object::Create<Boolean>(false);
  }
  return Object::Create<Boolean>(true);
}

Object *MoreOrEqual(std::shared_ptr<Scope> &,
                    const std::vector<Object *> &args) {
  for (size_t i = 1; i < args.size(); ++i) {
    if (!IsNumber(args[i]) || !IsNumber(args[i - 1]))
      throw RuntimeError("Syntax error!");

    if (AsNumber(args[i - 1])->GetValue() < AsNumber(args[i])->GetValue())
      return Object::Create<Boolean>(false);
  }
  return Object::Create<Boolean>(true);
}

Object *LessOrEqual(std::shared_ptr<Scope> &,
                    const std::vector<Object *> &args) {
  for (size_t i = 1; i < args.size(); ++i) {
    if (!IsNumber(args[i]) || !IsNumber(args[i - 1]))
      throw RuntimeError("Syntax error!");

    if (AsNumber(args[i - 1])->GetValue() > AsNumber(args[i])->GetValue())
      return Object::Create<Boolean>(false);
  }
  return Object::Create<Boolean>(true);
}

Object *Min(std::shared_ptr<Scope> &, const std::vector<Object *> &args) {
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
  return Object::Create<Number>(value);
}

Object *Max(std::shared_ptr<Scope> &, const std::vector<Object *> &args) {
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
  return Object::Create<Number>(value);
}

Object *Abs(std::shared_ptr<Scope> &, const std::vector<Object *> &args) {
  Function::CheckArgs(args, Kind::Allow, 1);

  if (!IsNumber(args[0]))
    throw RuntimeError("Syntax error!");

  return Object::Create<Number>(std::abs(AsNumber(args[0])->GetValue()));
}

Object *Cons(std::shared_ptr<Scope> &, const std::vector<Object *> &args) {
  Function::CheckArgs(args, Kind::Allow, 2);

  return Object::Create<Cell>(args[0], args[1]);
}

Object *Car(std::shared_ptr<Scope> &, const std::vector<Object *> &args) {
  Function::CheckArgs(args, Kind::Allow, 1);

  if (!IsCell(args[0]))
    throw RuntimeError("Syntax error!");

  return AsCell(args[0])->GetFirst();
}

Object *Cdr(std::shared_ptr<Scope> &, const std::vector<Object *> &args) {
  Function::CheckArgs(args, Kind::Allow, 1);

  if (!IsCell(args[0]))
    throw RuntimeError("Syntax error!");

  return AsCell(args[0])->GetSecond();
}

Object *SetCar(std::shared_ptr<Scope> &, const std::vector<Object *> &args) {
  Function::CheckArgs(args, Kind::Allow, 2);

  if (!IsCell(args[0]))
    throw RuntimeError("Syntax error!");

  AsCell(args[0])->SetFirst(args[1]);
  return args[0];
}

Object *SetCdr(std::shared_ptr<Scope> &, const std::vector<Object *> &args) {
  Function::CheckArgs(args, Kind::Allow, 2);

  if (!IsCell(args[0]))
    throw RuntimeError("Syntax error!");

  AsCell(args[0])->SetSecond(args[1]);
  return args[0];
}

Object *List(std::shared_ptr<Scope> &, const std::vector<Object *> &args) {
  if (args.size() == 0)
    return nullptr;

  auto new_cell = Object::Create<Cell>();
  auto res = new_cell;
  for (size_t ind = 0; ind < args.size();
       ++ind, new_cell = AsCell(new_cell->GetSecond())) {
    new_cell->SetFirst(args[ind]);
    if (ind != args.size() - 1)
      new_cell->SetSecond(Object::Create<Cell>());
  }
  return res;
}

Object *ListRef(std::shared_ptr<Scope> &, const std::vector<Object *> &args) {
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

Object *ListTail(std::shared_ptr<Scope> &, const std::vector<Object *> &args) {
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

Object *And(std::shared_ptr<Scope> &scope, const std::vector<Object *> &args) {
  for (size_t ind = 0; ind < args.size(); ++ind) {
    auto res = args[ind]->Eval(scope);
    if (res->IsFalse())
      return Object::Create<Boolean>(false);
    if (ind == args.size() - 1)
      return res;
  }
  return Object::Create<Boolean>(true);
}

Object *Or(std::shared_ptr<Scope> &scope, const std::vector<Object *> &args) {
  for (size_t ind = 0; ind < args.size(); ++ind) {
    auto res = args[ind]->Eval(scope);
    if (!res->IsFalse())
      return res;
  }
  return Object::Create<Boolean>(false);
}

Object *Define(std::shared_ptr<Scope> &scope,
               const std::vector<Object *> &args) {
  SpecialForm::CheckArgs(args, Kind::Allow, 2);

  if (IsSymbol(args[0])) {
    (*scope)[AsSymbol(args[0])] = args[1]->Eval(scope);
  } else if (IsCell(args[0])) {
    auto new_lambda_function = Object::Create<LambdaFunction>();
    new_lambda_function->SetArgs(ToVector(AsCell(args[0])->GetSecond()));
    for (auto &arg : new_lambda_function->GetArgs()) {
      if (!IsSymbol(arg))
        throw SyntaxError("wrong argument name");
    }
    new_lambda_function->SetScope(scope);
    for (size_t ind = 1; ind < args.size(); ++ind)
      new_lambda_function->AddToBody(args[ind]);

    (*scope)[AsSymbol(AsCell(args[0])->GetFirst())] = new_lambda_function;
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

  auto new_lambda_function = Object::Create<LambdaFunction>();
  new_lambda_function->SetArgs(ToVector(args[0]));
  for (auto &arg : new_lambda_function->GetArgs())
    if (!IsSymbol(arg))
      throw SyntaxError("wrong argument name");

  auto local_scope = Scope::Create(scope);
  new_lambda_function->SetScope(local_scope);
  for (size_t ind = 1; ind < args.size(); ++ind)
    new_lambda_function->AddToBody(args[ind]);
  return new_lambda_function;
}

void LambdaFunction::MarkRelated() {
  for (auto &arg : args_)
    arg->Mark();
  for (auto &body : body_)
    body->Mark();
}

void LambdaFunction::UnmarkRelated() {
  for (auto &arg : args_)
    arg->Unmark();
  for (auto &body : body_)
    body->Unmark();
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

Object *Exit(std::shared_ptr<Scope> &, const std::vector<Object *> &args) {
  Function::CheckArgs(args, Kind::Allow, 0);
  return Object::Create<BuiltInObject>();
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

Object *ReadProper(Tokenizer *tokenizer) {
  if (tokenizer->IsEnd())
    return nullptr;

  auto current_object = tokenizer->GetToken();
  if (SymbolToken *symbol = std::get_if<SymbolToken>(&current_object)) {
    tokenizer->Next();
    if (symbol->name == "#t")
      return Object::Create<Boolean>(true);
    else if (symbol->name == "#f")
      return Object::Create<Boolean>(false);
    return Object::Create<Symbol>(symbol->name);
  } else if (ConstantToken *constant =
                 std::get_if<ConstantToken>(&current_object)) {
    tokenizer->Next();
    return Object::Create<Number>(constant->value);
  } else if (std::holds_alternative<QuoteToken>(current_object)) {
    tokenizer->Next();
    auto new_cell = Object::Create<Cell>();
    auto list = Object::Create<Cell>();
    new_cell->SetFirst(Object::Create<Symbol>("quote"));
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
      auto new_cell = Object::Create<Cell>();
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
