#pragma once
#include "interfaces.h"
#include <cstddef>
#include <optional>

class SpecialForm : public Object, public Applicable {
public:
  using ApplyMethod = GCTracked *(*)(std::shared_ptr<Scope> &scope,
                                     const std::vector<GCTracked *> &);

  static SpecialForm *AllocIn(T *storage);

  SpecialForm(std::string name, ApplyMethod &&apply_method,
              std::optional<size_t> arg_min = std::nullopt,
              std::optional<size_t> arg_max = std::nullopt);

  void PrintTo(std::ostream *out) const override;

  Types ID() const override;

  virtual GCTracked *Apply(std::shared_ptr<Scope> &scope,
                           GCTracked *args) override;

protected:
  const std::string name_;
  std::optional<size_t> arg_min_;
  std::optional<size_t> arg_max_;
  const ApplyMethod apply_method_;
};