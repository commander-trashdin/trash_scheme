#pragma once
#include "interfaces.h"

class SpecialForm : public Object, public Applicable {
public:
  using ApplyMethod = Object *(*)(std::shared_ptr<Scope> &scope,
                                  const std::vector<Object *> &);

  SpecialForm(const std::string &&name, ApplyMethod &&apply_method);

  void PrintTo(std::ostream *out) const override;

  virtual Object *Apply(std::shared_ptr<Scope> &scope,
                        GCTracked *args) override;

  template <typename... Sizes>
  static void CheckArgs(const std::vector<Object *> &args, Kind kind,
                        Sizes... sizes);

protected:
  const std::string name;

  const ApplyMethod apply_method;
};