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

enum class Types { tType, cellType, numberType, symbolType };

enum class Kind { Allow, Disallow };
class Object;
class Symbol;
class Number;
class Boolean;
class GCManager;

struct constant {};

template <typename Derived>
concept DerivedFromObject = std::derived_from<Derived, Object>;

template <typename Tag>
concept ConstTag =
    std::is_same_v<Tag, void> || std::is_same_v<Tag, struct constant>;

template <typename Derived>
concept NumberOrSymbol =
    std::is_same_v<Derived, Number> || std::is_same_v<Derived, Symbol>;

template <typename Derived, typename Tag>
concept Creatable =
    (std::is_same_v<Tag, constant> && NumberOrSymbol<Derived>) ||
    !std::is_same_v<Tag, constant>;

class Scope : public std::enable_shared_from_this<Scope> {
public:
  static std::shared_ptr<Scope> Create();

  static std::shared_ptr<Scope> Create(std::shared_ptr<Scope> &parent);

  Scope() = default;
  explicit Scope(std::shared_ptr<Scope> &parent) : parent_(parent){};
  ~Scope();
  void Release();

  std::pair<Object *, std::shared_ptr<Scope>> Lookup(const std::string &name);

  Object *&operator[](Symbol *symbol);

  std::unordered_map<std::string, Object *> variables_;
  std::shared_ptr<Scope> parent_;
};

class Object {
public:
  enum class GCMark : int { White = 0, Black = 1, Safe = 2 };

  friend bool operator<(GCMark a, GCMark b) {
    return static_cast<int>(a) < static_cast<int>(b);
  }

  Object() = default;

  virtual ~Object();

  void Mark(GCMark mark = GCMark::Black);
  void Unmark(GCMark mark = GCMark::Black);
  bool isMarked() const;

  virtual void MarkRelated(GCMark mark = GCMark::Black);
  virtual void UnmarkRelated(GCMark mark = GCMark::Black);

  virtual Types ID() const;

  virtual bool IsFalse() const;

  virtual void PrintTo(std::ostream *out) const = 0;
  virtual void PrintDebug(std::ostream *out) const = 0;

  virtual Object *Eval(std::shared_ptr<Scope> &scope) = 0;

private:
  GCMark marked_ = GCMark::Black;
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

  virtual void MarkRelated(GCMark mark = GCMark::Black) override;
  virtual void UnmarkRelated(GCMark mark = GCMark::Black) override;

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
  using ValueType = int64_t;
  static std::unordered_map<ValueType, Number *> *GetConstantRegistry();

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
  using ApplyMethod = Object *(*)(std::shared_ptr<Scope> &scope,
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
  using ValueType = std::string_view;

  static std::unordered_map<std::string_view, Symbol *> *GetConstantRegistry();

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
  using ValueType = bool;

  Boolean();

  explicit Boolean(bool val);

  virtual Object *Eval(std::shared_ptr<Scope> &) override;

  bool IsFalse() const override;
};

class Function : public Object {
public:
  using ApplyMethod = Object *(*)(const std::vector<Object *> &);

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
  LambdaFunction(std::shared_ptr<Scope> scope, std::vector<Object *> &&args,
                 std::span<Object *const> body)
      : Function("", nullptr), current_scope_(scope), args_(args),
        body_(body.begin(), body.end()) {}

  virtual void MarkRelated(GCMark mark = GCMark::Black) override;
  virtual void UnmarkRelated(GCMark mark = GCMark::Black) override;

  Object *Apply(std::shared_ptr<Scope> &scope,
                const std::vector<Object *> &args) override;

  std::shared_ptr<Scope> GetScope() { return current_scope_; }

  void SetScope(const std::shared_ptr<Scope> &scope) { current_scope_ = scope; }

  std::vector<Object *> GetArgs() { return args_; }

  void SetArgs(std::vector<Object *> args) { args_ = args; }

  std::vector<Object *> GetBody() { return body_; }

  void AddToBody(Object *form) { body_.push_back(form); }

  void Release();

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

template <DerivedFromObject Derived> Derived *Is(Object *obj) {
  return dynamic_cast<Derived *>(obj);
}

bool IsNumber(const Object *obj);
Number *AsNumber(const Object *obj);

bool IsCell(const Object *obj);
Cell *AsCell(const Object *obj);

bool IsSymbol(const Object *obj);
Symbol *AsSymbol(const Object *obj);

bool IsCell(const Object *obj);
Cell *AsCell(const Object *obj);

bool IsFunction(const Object *obj);
Function *AsFunction(const Object *obj);

Object *Quote(std::shared_ptr<Scope> &scope, const std::vector<Object *> &args);

Object *Plus(const std::vector<Object *> &args);

Object *Minus(const std::vector<Object *> &args);

Object *Multiply(const std::vector<Object *> &args);

Object *Divide(const std::vector<Object *> &args);

Object *If(std::shared_ptr<Scope> &scope, const std::vector<Object *> &args);

Object *CheckNull(const std::vector<Object *> &args);

Object *CheckPair(const std::vector<Object *> &args);

Object *CheckNumber(const std::vector<Object *> &args);
Object *CheckBoolean(const std::vector<Object *> &args);

Object *CheckSymbol(const std::vector<Object *> &args);

Object *CheckList(const std::vector<Object *> &args);

// FIXME
Object *Eq(const std::vector<Object *> &args);
// FIXME
Object *Equal(const std::vector<Object *> &args);
Object *IntegerEqual(const std::vector<Object *> &args);

Object *Not(const std::vector<Object *> &args);

Object *Equality(const std::vector<Object *> &args);

Object *More(const std::vector<Object *> &args);

Object *Less(const std::vector<Object *> &args);

Object *MoreOrEqual(const std::vector<Object *> &args);

Object *LessOrEqual(const std::vector<Object *> &args);

Object *Min(const std::vector<Object *> &args);

Object *Max(const std::vector<Object *> &args);

Object *Abs(const std::vector<Object *> &args);

Object *Cons(const std::vector<Object *> &args);

Object *Car(const std::vector<Object *> &args);

Object *Cdr(const std::vector<Object *> &args);

Object *SetCar(const std::vector<Object *> &args);

Object *SetCdr(const std::vector<Object *> &args);

Object *List(const std::vector<Object *> &args);

Object *ListRef(const std::vector<Object *> &args);

Object *ListTail(const std::vector<Object *> &args);

Object *And(std::shared_ptr<Scope> &scope, const std::vector<Object *> &args);

Object *Or(std::shared_ptr<Scope> &scope, const std::vector<Object *> &args);

Object *Define(std::shared_ptr<Scope> &scope,
               const std::vector<Object *> &args);

Object *Set(std::shared_ptr<Scope> &scope, const std::vector<Object *> &args);

Object *Lambda(std::shared_ptr<Scope> &scope,
               const std::vector<Object *> &args);

Object *Exit(const std::vector<Object *> &args);

Object *Map(const std::vector<Object *> &args);

// Object* Load(const std::vector<Object*> &args);

class Parser {
public:
  explicit Parser(Tokenizer &&tok);

  Object *ReadList();

  Object *Read();

  Object *ReadProper();

private:
  void ParenClose();
  void ParenOpen();
  Tokenizer tokenizer_;
  std::unordered_map<std::string_view, Symbol *> sym_table_;
  int64_t paren_count_ = 0;
};
