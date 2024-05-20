#pragma once
#include "interfaces.h"
#include "scope.h"
#include <optional>
#include <span>
#include <variant>
#include <vector>

struct BuiltInFunction {};

class Function : public Object, public Applicable {
public:
  using ApplyMethod = GCTracked *(*)(std::shared_ptr<Scope> &scope,
                                     const std::vector<GCTracked *> &);
  static Function *AllocIn(T *storage);
  Function(std::string name, std::variant<Types, std::vector<Types>> arg_types,
           ApplyMethod &&apply_method);

  void PrintTo(std::ostream *out) const override;

  [[nodiscard]] Types ID() const override;

  GCTracked *Apply(std::shared_ptr<Scope> &scope, GCTracked *args) override;

  std::optional<Types> ArgType(size_t index);

protected:
  std::string name;
  std::variant<Types, std::vector<Types>> arg_types_;

  const ApplyMethod apply_method;
};

class LambdaFunction : public Object, public Applicable {
public:
  LambdaFunction(std::shared_ptr<Scope> scope, std::vector<GCTracked *> &&args,
                 std::span<GCTracked *const> body);

  static LambdaFunction *AllocIn(T *storage);
  GCTracked *Apply(std::shared_ptr<Scope> &scope, GCTracked *args) override;

  [[nodiscard]] Types ID() const override;

  void PrintTo(std::ostream *out) const override;

  std::shared_ptr<Scope> GetScope();

  void SetScope(const std::shared_ptr<Scope> &scope);

  std::vector<GCTracked *> GetArgs();

  void SetArgs(std::vector<GCTracked *> args);

  std::vector<GCTracked *> GetBody();

  void AddToBody(GCTracked *form);

  void Walk(const std::function<void(GCTracked *)> &fn) override;

private:
  std::shared_ptr<Scope> current_scope_;
  std::vector<GCTracked *> args_;
  std::vector<GCTracked *> body_;
};
