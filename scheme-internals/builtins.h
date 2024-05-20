#include "interfaces.h"
#include "scope.h"

GCTracked *Quote(std::shared_ptr<Scope> &scope,
                 const std::vector<GCTracked *> &args);

GCTracked *If(std::shared_ptr<Scope> &scope,
              const std::vector<GCTracked *> &args);

GCTracked *And(std::shared_ptr<Scope> &scope,
               const std::vector<GCTracked *> &args);

GCTracked *Or(std::shared_ptr<Scope> &scope,
              const std::vector<GCTracked *> &args);

GCTracked *Define(std::shared_ptr<Scope> &scope,
                  const std::vector<GCTracked *> &args);

GCTracked *Set(std::shared_ptr<Scope> &scope,
               const std::vector<GCTracked *> &args);

GCTracked *Lambda(std::shared_ptr<Scope> &scope,
                  const std::vector<GCTracked *> &args);

GCTracked *Eval(std::shared_ptr<Scope> &scope,
                const std::vector<GCTracked *> &args);

GCTracked *Plus(std::shared_ptr<Scope> &scope,
                const std::vector<GCTracked *> &args);

GCTracked *Minus(std::shared_ptr<Scope> &scope,
                 const std::vector<GCTracked *> &args);

GCTracked *Multiply(std::shared_ptr<Scope> &scope,
                    const std::vector<GCTracked *> &args);

GCTracked *Divide(std::shared_ptr<Scope> &scope,
                  const std::vector<GCTracked *> &args);

GCTracked *CheckNull(std::shared_ptr<Scope> &scope,
                     const std::vector<GCTracked *> &args);

GCTracked *CheckPair(std::shared_ptr<Scope> &scope,
                     const std::vector<GCTracked *> &args);

GCTracked *CheckNumber(std::shared_ptr<Scope> &scope,
                       const std::vector<GCTracked *> &args);
GCTracked *CheckBoolean(std::shared_ptr<Scope> &scope,
                        const std::vector<GCTracked *> &args);

GCTracked *CheckSymbol(std::shared_ptr<Scope> &scope,
                       const std::vector<GCTracked *> &args);

GCTracked *CheckList(std::shared_ptr<Scope> &scope,
                     const std::vector<GCTracked *> &args);

GCTracked *Eq(std::shared_ptr<Scope> &scope,
              const std::vector<GCTracked *> &args);
GCTracked *Eql(std::shared_ptr<Scope> &scope,
               const std::vector<GCTracked *> &args);

GCTracked *Not(std::shared_ptr<Scope> &scope,
               const std::vector<GCTracked *> &args);

GCTracked *Equality(std::shared_ptr<Scope> &scope,
                    const std::vector<GCTracked *> &args);

GCTracked *More(std::shared_ptr<Scope> &scope,
                const std::vector<GCTracked *> &args);

GCTracked *Less(std::shared_ptr<Scope> &scope,
                const std::vector<GCTracked *> &args);

GCTracked *MoreOrEqual(std::shared_ptr<Scope> &scope,
                       const std::vector<GCTracked *> &args);

GCTracked *LessOrEqual(std::shared_ptr<Scope> &scope,
                       const std::vector<GCTracked *> &args);

GCTracked *Min(std::shared_ptr<Scope> &scope,
               const std::vector<GCTracked *> &args);

GCTracked *Max(std::shared_ptr<Scope> &scope,
               const std::vector<GCTracked *> &args);

GCTracked *Cons(std::shared_ptr<Scope> &scope,
                const std::vector<GCTracked *> &args);

GCTracked *Car(std::shared_ptr<Scope> &scope,
               const std::vector<GCTracked *> &args);

GCTracked *Cdr(std::shared_ptr<Scope> &scope,
               const std::vector<GCTracked *> &args);

GCTracked *SetCar(std::shared_ptr<Scope> &scope,
                  const std::vector<GCTracked *> &args);

GCTracked *SetCdr(std::shared_ptr<Scope> &scope,
                  const std::vector<GCTracked *> &args);

GCTracked *List(std::shared_ptr<Scope> &scope,
                const std::vector<GCTracked *> &args);

GCTracked *ListRef(std::shared_ptr<Scope> &scope,
                   const std::vector<GCTracked *> &args);

GCTracked *ListTail(std::shared_ptr<Scope> &scope,
                    const std::vector<GCTracked *> &args);

GCTracked *Exit(std::shared_ptr<Scope> &scope,
                const std::vector<GCTracked *> &args);

GCTracked *Map(std::shared_ptr<Scope> &scope,
               const std::vector<GCTracked *> &args);

GCTracked *Load(std::shared_ptr<Scope> &scope,
                const std::vector<GCTracked *> &args);

GCTracked *Print(std::shared_ptr<Scope> &scope,
                 const std::vector<GCTracked *> &args);

GCTracked *Read(std::shared_ptr<Scope> &scope,
                const std::vector<GCTracked *> &args);