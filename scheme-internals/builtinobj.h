#include "interfaces.h"

class BuiltInObject : public Object {
public:
  static BuiltInObject *AllocIn(T *storage);

  virtual Types ID() const override;

  virtual void PrintTo(std::ostream *out) const override;
};