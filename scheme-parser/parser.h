#pragma once

#include "../scheme-tokenizer/tokenizer.h"
#include "gc.h"
#include <memory>
#include <string>
#include <sys/stat.h>
#include <unordered_map>
#include <vector>

enum class Types { tType, cellType, numberType, symbolType };

enum class Kind { Allow, Disallow };

class Symbol;

class Scope : public std::enable_shared_from_this<Scope> {
public:
  static std::shared_ptr<Scope> Create() {
    std::shared_ptr<Scope> newScope = std::make_shared<Scope>();
    GCManager::GetInstance().AddRoot(newScope);
    return newScope;
  }

  static std::shared_ptr<Scope> Create(std::shared_ptr<Scope> &parent) {
    std::shared_ptr<Scope> newScope = std::make_shared<Scope>(parent);
    GCManager::GetInstance().AddRoot(newScope);
    return newScope;
  }

  Scope() = default;
  explicit Scope(std::shared_ptr<Scope> &parent) : parent_(parent){};
  ~Scope();

  Object *Lookup(const std::string &name);

  Object *&operator[](Symbol *symbol);

  std::unordered_map<std::string, Object *> variables_;
  std::shared_ptr<Scope> parent_;
};

class Object {
public:
  Object() = default;

  template <class Derived, typename... Args>
  static Derived *Create(Args &&...args) {
    Derived *obj = new Derived(std::forward<Args>(args)...);
    GCManager::GetInstance().RegisterObject(obj);
    return obj;
  }

  virtual ~Object();

  void Mark();
  void Unmark();
  bool isMarked() const;

  virtual void MarkRelated();
  virtual void UnmarkRelated();

  virtual Types ID() const;

  virtual bool IsFalse() const;

  virtual void PrintTo(std::ostream *out) const = 0;
  virtual void PrintDebug(std::ostream *out) const = 0;

  virtual Object *Eval(std::shared_ptr<Scope> &scope) = 0;

private:
  bool marked_ = false;
};

class BuiltInObject : public Object {
public:
  virtual Types ID() const override;

  virtual void PrintTo(std::ostream *out) const override;
  virtual void PrintDebug(std::ostream *out) const override;

  virtual Object *Eval(std::shared_ptr<Scope> &scope) override;
};

class Cell : public Object {
public:
  Cell();

  Cell(Object *head, Object *tail);

  virtual void MarkRelated() override;
  virtual void UnmarkRelated() override;

  virtual Types ID() const override;

  virtual void PrintTo(std::ostream *out) const override;
  virtual void PrintDebug(std::ostream *out) const override;

  virtual Object *Eval(std::shared_ptr<Scope> &scope) override;

  Object *GetFirst() const;

  void SetFirst(Object *object);

  Object *GetSecond() const;

  void SetSecond(Object *object);

private:
  Object *head_;
  Object *tail_;
};

class Number : public Object {
public:
  Number();

  explicit Number(int64_t value);

  virtual Types ID() const override;

  virtual void PrintTo(std::ostream *out) const override;
  virtual void PrintDebug(std::ostream *out) const override;

  virtual Object *Eval(std::shared_ptr<Scope> &) override;

  int64_t GetValue() const;

  int64_t &SetValue();

private:
  int64_t value_;
};

class SpecialForm : public Object {
public:
  using ApplyMethod = Object *(*)(std::shared_ptr<Scope> &,
                                  const std::vector<Object *> &);

  SpecialForm(const std::string &&name, ApplyMethod &&apply_method);

  virtual Object *Eval(std::shared_ptr<Scope> &scope) override;

  void PrintTo(std::ostream *out) const override;
  virtual void PrintDebug(std::ostream *out) const override;

  virtual Object *Apply(std::shared_ptr<Scope> &scope,
                        const std::vector<Object *> &args);

  template <typename... Sizes>
  static void CheckArgs(const std::vector<Object *> &args, Kind kind,
                        Sizes... sizes);

protected:
  std::string name;

  const ApplyMethod apply_method;
};

class Symbol : public Object {
public:
  Symbol();

  explicit Symbol(std::string name);

  virtual Types ID() const override;

  virtual void PrintTo(std::ostream *out) const override;
  virtual void PrintDebug(std::ostream *out) const override;

  virtual Object *Eval(std::shared_ptr<Scope> &scope) override;

  const std::string &GetName() const;

  std::string &SetName();

protected:
  std::string name_;
};

class Boolean : public Symbol {
public:
  Boolean();

  explicit Boolean(bool val);

  virtual Object *Eval(std::shared_ptr<Scope> &) override;

  bool IsFalse() const override;
};

class Function : public Object {
public:
  using ApplyMethod = Object *(*)(std::shared_ptr<Scope> &,
                                  const std::vector<Object *> &);

  Function(const std::string &&name, ApplyMethod &&apply_method);

  virtual Object *Eval(std::shared_ptr<Scope> &scope) override;

  void PrintTo(std::ostream *out) const override;
  virtual void PrintDebug(std::ostream *out) const override;

  virtual Object *Apply(std::shared_ptr<Scope> &scope,
                        const std::vector<Object *> &args);

  template <typename... Sizes>
  static void CheckArgs(const std::vector<Object *> &args, Kind kind,
                        Sizes... sizes);

protected:
  std::string name;

  const ApplyMethod apply_method;
};

class LambdaFunction : public Function {
public:
  LambdaFunction() : Function("", nullptr){};
  virtual void MarkRelated() override;
  virtual void UnmarkRelated() override;

  Object *Apply(std::shared_ptr<Scope> &scope,
                const std::vector<Object *> &args) override;

  std::shared_ptr<Scope> GetScope() { return current_scope_; }

  void SetScope(const std::shared_ptr<Scope> &scope) { current_scope_ = scope; }

  std::vector<Object *> GetArgs() { return args_; }

  void SetArgs(std::vector<Object *> args) { args_ = args; }

  std::vector<Object *> GetBody() { return body_; }

  void AddToBody(Object *form) { body_.push_back(form); }

private:
  std::shared_ptr<Scope> current_scope_;
  std::vector<Object *> args_;
  std::vector<Object *> body_;
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

inline void PrintTo(const Object *obj, std::ostream *out) {
  if (!obj) {
    *out << "()";
    return;
  }
  obj->PrintTo(out);
}

std::vector<Object *> ToVector(const Object *head);

inline std::string Print(const Object *obj);

bool IsNumber(const Object *obj);
Number *AsNumber(const Object *obj);

bool IsCell(const Object *obj);
Cell *AsCell(const Object *obj);

bool IsSymbol(const Object *obj);
Symbol *AsSymbol(const Object *obj);

Object *ReadList(Tokenizer *tokenizer);

Object *Read(Tokenizer *tokenizer);

bool IsCell(const Object *obj);
Cell *AsCell(const Object *obj);

Object *Quote(std::shared_ptr<Scope> &scope, const std::vector<Object *> &args);

Object *Plus(std::shared_ptr<Scope> &, const std::vector<Object *> &args);

Object *Minus(std::shared_ptr<Scope> &, const std::vector<Object *> &args);

Object *Multiply(std::shared_ptr<Scope> &, const std::vector<Object *> &args);

Object *Divide(std::shared_ptr<Scope> &, const std::vector<Object *> &args);

Object *If(std::shared_ptr<Scope> &scope, const std::vector<Object *> &args);

Object *CheckNull(std::shared_ptr<Scope> &, const std::vector<Object *> &args);

Object *CheckPair(std::shared_ptr<Scope> &, const std::vector<Object *> &args);

Object *CheckNumber(std::shared_ptr<Scope> &,
                    const std::vector<Object *> &args);
Object *CheckBoolean(std::shared_ptr<Scope> &,
                     const std::vector<Object *> &args);

Object *CheckSymbol(std::shared_ptr<Scope> &,
                    const std::vector<Object *> &args);

Object *CheckList(std::shared_ptr<Scope> &, const std::vector<Object *> &args);

// FIXME
Object *Eq(std::shared_ptr<Scope> &, const std::vector<Object *> &args);
// FIXME
Object *Equal(std::shared_ptr<Scope> &, const std::vector<Object *> &args);
Object *IntegerEqual(std::shared_ptr<Scope> &,
                     const std::vector<Object *> &args);

Object *Not(std::shared_ptr<Scope> &, const std::vector<Object *> &args);

Object *Equality(std::shared_ptr<Scope> &, const std::vector<Object *> &args);

Object *More(std::shared_ptr<Scope> &, const std::vector<Object *> &args);

Object *Less(std::shared_ptr<Scope> &, const std::vector<Object *> &args);

Object *MoreOrEqual(std::shared_ptr<Scope> &,
                    const std::vector<Object *> &args);

Object *LessOrEqual(std::shared_ptr<Scope> &,
                    const std::vector<Object *> &args);

Object *Min(std::shared_ptr<Scope> &, const std::vector<Object *> &args);

Object *Max(std::shared_ptr<Scope> &, const std::vector<Object *> &args);

Object *Abs(std::shared_ptr<Scope> &, const std::vector<Object *> &args);

Object *Cons(std::shared_ptr<Scope> &, const std::vector<Object *> &args);

Object *Car(std::shared_ptr<Scope> &, const std::vector<Object *> &args);

Object *Cdr(std::shared_ptr<Scope> &, const std::vector<Object *> &args);

Object *SetCar(std::shared_ptr<Scope> &, const std::vector<Object *> &args);

Object *SetCdr(std::shared_ptr<Scope> &, const std::vector<Object *> &args);

Object *List(std::shared_ptr<Scope> &, const std::vector<Object *> &args);

Object *ListRef(std::shared_ptr<Scope> &, const std::vector<Object *> &args);

Object *ListTail(std::shared_ptr<Scope> &, const std::vector<Object *> &args);

Object *And(std::shared_ptr<Scope> &scope, const std::vector<Object *> &args);

Object *Or(std::shared_ptr<Scope> &scope, const std::vector<Object *> &args);

Object *Define(std::shared_ptr<Scope> &scope,
               const std::vector<Object *> &args);

Object *Set(std::shared_ptr<Scope> &scope, const std::vector<Object *> &args);

Object *Lambda(std::shared_ptr<Scope> &scope,
               const std::vector<Object *> &args);

Object *Exit(std::shared_ptr<Scope> &, const std::vector<Object *> &args);