#include "interfaces.h"

class BuiltInObject : public Object {
public:
  virtual Types ID() const override;

  virtual void PrintTo(std::ostream *out) const override;
};

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
