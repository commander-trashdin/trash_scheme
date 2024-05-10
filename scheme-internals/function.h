#pragma once
#include "interfaces.h"

class Function : public Object, public Applicable {
public:
  using ApplyMethod = Object *(*)(const std::vector<Object *> &);

  Function(const std::string &&name, ApplyMethod &&apply_method);

  void PrintTo(std::ostream *out) const override;

  virtual Object *Apply(std::shared_ptr<Scope> &scope,
                        GCTracked *args) override;

  template <typename... Sizes>
  static void CheckArgs(const std::vector<Object *> &args, Kind kind,
                        Sizes... sizes);

protected:
  std::string name;

  const ApplyMethod apply_method;
};

class LambdaFunction : public Object, public Applicable {
public:
  LambdaFunction(std::shared_ptr<Scope> scope, std::vector<Object *> &&args,
                 std::span<Object *const> body)
      : current_scope_(scope), args_(args), body_(body.begin(), body.end()) {}

  Object *Apply(std::shared_ptr<Scope> &scope, GCTracked *args) override;

  void PrintTo(std::ostream *out) const override;

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
