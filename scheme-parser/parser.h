#pragma once

#include "../scheme-tokenizer/tokenizer.h"
#include <algorithm>
#include <memory>
#include <unordered_map>
#include <vector>

enum class Types {
  tType,
  cellType,
  numberType,
  symbolType,
  quoteType,
  dotType
};

enum class Kind { Allow, Disallow };

class Object;
class Symbol;

class Scope {
public:
  Scope();

  explicit Scope(std::shared_ptr<Scope> &parent);

  std::shared_ptr<Object> Lookup(const std::string &name);

  std::shared_ptr<Object> &operator[](const std::shared_ptr<Symbol> &symbol);

  std::unordered_map<std::string, std::shared_ptr<Object>> variables_;
  std::shared_ptr<Scope> parent_;
};

class Object : public std::enable_shared_from_this<Object> {
public:
  virtual Types ID() const;

  virtual bool IsFalse();

  virtual void PrintTo(std::ostream *out) = 0;

  virtual std::shared_ptr<Object> Eval(std::shared_ptr<Scope> &scope) = 0;

  virtual ~Object();
};

class BuiltInObject : public Object {
public:
  virtual Types ID() const override;

  virtual void PrintTo(std::ostream *out) override;

  virtual std::shared_ptr<Object> Eval(std::shared_ptr<Scope> &scope) override;
};

class Cell : public Object {
public:
  Cell();

  Cell(std::shared_ptr<Object> head, std::shared_ptr<Object> tail);

  virtual Types ID() const override;

  virtual void PrintTo(std::ostream *out) override;

  virtual std::shared_ptr<Object> Eval(std::shared_ptr<Scope> &scope) override;

  const std::shared_ptr<Object> &GetFirst() const;

  void SetFirst(std::shared_ptr<Object> object);

  const std::shared_ptr<Object> &GetSecond() const;

  void SetSecond(std::shared_ptr<Object> object);

private:
  std::shared_ptr<Object> head_;
  std::shared_ptr<Object> tail_;
};

class Number : public Object {
public:
  Number();

  explicit Number(int64_t value);

  virtual Types ID() const override;

  virtual void PrintTo(std::ostream *out) override;

  virtual std::shared_ptr<Object> Eval(std::shared_ptr<Scope> &) override;

  int64_t GetValue() const;

  int64_t &SetValue();

private:
  int64_t value_;
};

class SpecialForm : public Object {
public:
  virtual std::shared_ptr<Object> Eval(std::shared_ptr<Scope> &scope) override;

  void PrintTo(std::ostream *out) override;

  virtual std::shared_ptr<Object>
  Apply(std::shared_ptr<Scope> &scope,
        const std::vector<std::shared_ptr<Object>> &args) = 0;

protected:
  template <typename... Sizes>
  void CheckArgs(const std::vector<std::shared_ptr<Object>> &args, Kind kind,
                 Sizes... sizes);
};

class Symbol : public Object {
public:
  Symbol();

  explicit Symbol(std::string name);

  virtual Types ID() const override;

  virtual void PrintTo(std::ostream *out) override;

  virtual std::shared_ptr<Object> Eval(std::shared_ptr<Scope> &scope) override;

  const std::string &GetName() const;

  std::string &SetName();

protected:
  std::string name_;
};

class Boolean : public Symbol {
public:
  Boolean();

  explicit Boolean(bool val);

  virtual std::shared_ptr<Object> Eval(std::shared_ptr<Scope> &) override;

  bool IsFalse() override;
};

class Quote : public SpecialForm {
public:
  virtual Types ID() const override;

  std::shared_ptr<Object>
  Apply(std::shared_ptr<Scope> &scope,
        const std::vector<std::shared_ptr<Object>> &args) override;
};

class Dot : public Object {
  virtual Types ID() const override;
};

class Function : public Object {
public:
  virtual std::shared_ptr<Object> Eval(std::shared_ptr<Scope> &scope) override;

  void PrintTo(std::ostream *out) override;

  virtual std::shared_ptr<Object>
  Apply(std::shared_ptr<Scope> &scope,
        const std::vector<std::shared_ptr<Object>> &args) = 0;

protected:
  template <typename... Sizes>
  void CheckArgs(const std::vector<std::shared_ptr<Object>> &args, Kind kind,
                 Sizes... sizes);
};

class Plus : public Function {
public:
  std::shared_ptr<Object>
  Apply(std::shared_ptr<Scope> &scope,
        const std::vector<std::shared_ptr<Object>> &args) override;
};

class Minus : public Function {
public:
  std::shared_ptr<Object>
  Apply(std::shared_ptr<Scope> &scope,
        const std::vector<std::shared_ptr<Object>> &args) override;
};

class Multiply : public Function {
public:
  std::shared_ptr<Object>
  Apply(std::shared_ptr<Scope> &scope,
        const std::vector<std::shared_ptr<Object>> &args) override;
};

class Divide : public Function {
public:
  std::shared_ptr<Object>
  Apply(std::shared_ptr<Scope> &scope,
        const std::vector<std::shared_ptr<Object>> &args) override;
};

class If : public SpecialForm {
public:
  std::shared_ptr<Object>
  Apply(std::shared_ptr<Scope> &scope,
        const std::vector<std::shared_ptr<Object>> &args) override;
};

class CheckNull : public Function {
public:
  std::shared_ptr<Object>
  Apply(std::shared_ptr<Scope> &scope,
        const std::vector<std::shared_ptr<Object>> &args) override;
};

class CheckPair : public Function {
public:
  std::shared_ptr<Object>
  Apply(std::shared_ptr<Scope> &scope,
        const std::vector<std::shared_ptr<Object>> &args) override;
};

class CheckNumber : public Function {
public:
  std::shared_ptr<Object>
  Apply(std::shared_ptr<Scope> &scope,
        const std::vector<std::shared_ptr<Object>> &args) override;
};

class CheckBoolean : public Function {
public:
  std::shared_ptr<Object>
  Apply(std::shared_ptr<Scope> &scope,
        const std::vector<std::shared_ptr<Object>> &args) override;
};

class CheckSymbol : public Function {
public:
  std::shared_ptr<Object>
  Apply(std::shared_ptr<Scope> &scope,
        const std::vector<std::shared_ptr<Object>> &args) override;
};

class CheckList : public Function {
public:
  std::shared_ptr<Object>
  Apply(std::shared_ptr<Scope> &scope,
        const std::vector<std::shared_ptr<Object>> &args) override;
};

class Eq : public Function {
public:
  std::shared_ptr<Object>
  Apply(std::shared_ptr<Scope> &scope,
        const std::vector<std::shared_ptr<Object>> &args) override;
};

class Equal : public Function {
public:
  std::shared_ptr<Object>
  Apply(std::shared_ptr<Scope> &scope,
        const std::vector<std::shared_ptr<Object>> &args) override;
};

class IntegerEqual : public Function {
public:
  std::shared_ptr<Object>
  Apply(std::shared_ptr<Scope> &scope,
        const std::vector<std::shared_ptr<Object>> &args) override;
};

class Not : public Function {
public:
  std::shared_ptr<Object>
  Apply(std::shared_ptr<Scope> &scope,
        const std::vector<std::shared_ptr<Object>> &args) override;
};

class Equality : public Function {
public:
  std::shared_ptr<Object>
  Apply(std::shared_ptr<Scope> &scope,
        const std::vector<std::shared_ptr<Object>> &args) override;
};

class More : public Function {
public:
  std::shared_ptr<Object>
  Apply(std::shared_ptr<Scope> &scope,
        const std::vector<std::shared_ptr<Object>> &args) override;
};

class Less : public Function {
public:
  std::shared_ptr<Object>
  Apply(std::shared_ptr<Scope> &scope,
        const std::vector<std::shared_ptr<Object>> &args) override;
};

class MoreOrEqual : public Function {
public:
  std::shared_ptr<Object>
  Apply(std::shared_ptr<Scope> &scope,
        const std::vector<std::shared_ptr<Object>> &args) override;
};

class LessOrEqual : public Function {
public:
  std::shared_ptr<Object>
  Apply(std::shared_ptr<Scope> &scope,
        const std::vector<std::shared_ptr<Object>> &args) override;
};

class Min : public Function {
public:
  std::shared_ptr<Object>
  Apply(std::shared_ptr<Scope> &scope,
        const std::vector<std::shared_ptr<Object>> &args) override;
};

class Max : public Function {
public:
  std::shared_ptr<Object>
  Apply(std::shared_ptr<Scope> &scope,
        const std::vector<std::shared_ptr<Object>> &args) override;
};

class Abs : public Function {
public:
  std::shared_ptr<Object>
  Apply(std::shared_ptr<Scope> &scope,
        const std::vector<std::shared_ptr<Object>> &args) override;
};

class Cons : public Function {
public:
  std::shared_ptr<Object>
  Apply(std::shared_ptr<Scope> &scope,
        const std::vector<std::shared_ptr<Object>> &args) override;
};

class Car : public Function {
public:
  std::shared_ptr<Object>
  Apply(std::shared_ptr<Scope> &scope,
        const std::vector<std::shared_ptr<Object>> &args) override;
};

class Cdr : public Function {
public:
  std::shared_ptr<Object>
  Apply(std::shared_ptr<Scope> &scope,
        const std::vector<std::shared_ptr<Object>> &args) override;
};

class SetCar : public Function {
public:
  std::shared_ptr<Object>
  Apply(std::shared_ptr<Scope> &scope,
        const std::vector<std::shared_ptr<Object>> &args) override;
};

class SetCdr : public Function {
public:
  std::shared_ptr<Object>
  Apply(std::shared_ptr<Scope> &scope,
        const std::vector<std::shared_ptr<Object>> &args) override;
};

class List : public Function {
public:
  std::shared_ptr<Object>
  Apply(std::shared_ptr<Scope> &scope,
        const std::vector<std::shared_ptr<Object>> &args) override;
};

class ListRef : public Function {
public:
  std::shared_ptr<Object>
  Apply(std::shared_ptr<Scope> &scope,
        const std::vector<std::shared_ptr<Object>> &args) override;
};

class ListTail : public Function {
public:
  std::shared_ptr<Object>
  Apply(std::shared_ptr<Scope> &scope,
        const std::vector<std::shared_ptr<Object>> &args) override;
};

class Exit : public Function {
public:
  std::shared_ptr<Object>
  Apply(std::shared_ptr<Scope> &scope,
        const std::vector<std::shared_ptr<Object>> &args) override;
};

class Lambda : public SpecialForm {
public:
  std::shared_ptr<Object>
  Apply(std::shared_ptr<Scope> &scope,
        const std::vector<std::shared_ptr<Object>> &args) override;
};

class LambdaFunction : public Function {
public:
  std::shared_ptr<Object>
  Apply(std::shared_ptr<Scope> &scope,
        const std::vector<std::shared_ptr<Object>> &args) override;

  std::shared_ptr<Scope> GetScope() { return current_scope_; }

  void SetScope(const std::shared_ptr<Scope> &scope) { current_scope_ = scope; }

  std::vector<std::shared_ptr<Object>> GetArgs() { return args_; }

  void SetArgs(std::vector<std::shared_ptr<Object>> args) { args_ = args; }

  std::vector<std::shared_ptr<Object>> GetBody() { return body_; }

  void AddToBody(std::shared_ptr<Object> form) { body_.push_back(form); }

private:
  std::shared_ptr<Scope> current_scope_;
  std::vector<std::shared_ptr<Object>> args_;
  std::vector<std::shared_ptr<Object>> body_;
};

class And : public SpecialForm {
public:
  std::shared_ptr<Object>
  Apply(std::shared_ptr<Scope> &scope,
        const std::vector<std::shared_ptr<Object>> &args) override;
};

class Or : public SpecialForm {
public:
  std::shared_ptr<Object>
  Apply(std::shared_ptr<Scope> &scope,
        const std::vector<std::shared_ptr<Object>> &args) override;
};

class Define : public SpecialForm {
public:
  std::shared_ptr<Object>
  Apply(std::shared_ptr<Scope> &scope,
        const std::vector<std::shared_ptr<Object>> &args) override;
};

class Set : public SpecialForm {
public:
  std::shared_ptr<Object>
  Apply(std::shared_ptr<Scope> &scope,
        const std::vector<std::shared_ptr<Object>> &args) override;
};

struct SyntaxError : public std::runtime_error {
  explicit SyntaxError(const std::string &what);
};

struct RuntimeError : public std::runtime_error {
  explicit RuntimeError(const std::string &what);
};

struct NameError : public std::runtime_error {
  explicit NameError(const std::string &what);
};

inline void PrintTo(const std::shared_ptr<Object> &obj, std::ostream *out) {
  if (!obj) {
    *out << "()";
    return;
  }
  obj->PrintTo(out);
}

std::vector<std::shared_ptr<Object>>
ToVector(const std::shared_ptr<Object> &head);

inline std::string Print(const std::shared_ptr<Object> &obj);

bool IsNumber(const std::shared_ptr<Object> &obj);

std::shared_ptr<Number> AsNumber(const std::shared_ptr<Object> &obj);

bool IsCell(const std::shared_ptr<Object> &obj);

std::shared_ptr<Cell> AsCell(const std::shared_ptr<Object> &obj);

bool IsSymbol(const std::shared_ptr<Object> &obj);

std::shared_ptr<Symbol> AsSymbol(const std::shared_ptr<Object> &obj);

std::shared_ptr<Object> ReadList(Tokenizer *tokenizer);

std::shared_ptr<Object> Read(Tokenizer *tokenizer);

bool IsCell(const std::shared_ptr<Object> &obj);

std::shared_ptr<Cell> AsCell(const std::shared_ptr<Object> &obj);
