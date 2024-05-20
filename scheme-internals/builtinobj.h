#include "interfaces.h"

class BuiltInObject : public Object {
public:
  static BuiltInObject *AllocIn(T *storage);

  [[nodiscard]] Types ID() const override;

  void PrintTo(std::ostream *out) const override;
};