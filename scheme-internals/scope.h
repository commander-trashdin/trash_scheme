#pragma once
#include "interfaces.h"

struct GCPtrSymHash {
  std::size_t operator()(GCTracked *k) const;
};

struct GCPtrSymEqual {
  bool operator()(GCTracked *lhs, GCTracked *rhs) const;
};

class Scope : public std::enable_shared_from_this<Scope> {
public:
  friend class GCManager;
  friend class LambdaFunction;

  static std::shared_ptr<Scope> Create();

  static std::shared_ptr<Scope> Create(std::shared_ptr<Scope> &parent);

  Scope() = default;
  explicit Scope(std::shared_ptr<Scope> &parent) : parent_(parent) {};
  ~Scope();

  std::pair<GCTracked *, std::shared_ptr<Scope>> Lookup(GCTracked *sym);

  std::shared_ptr<Scope> GetGlobalScope();

  GCTracked *&operator[](GCTracked *symbol);
  void Clear() { variables_.clear(); }

  auto contbegin() { return variables_.begin(); }
  auto contend() { return variables_.end(); }

private:
  std::unordered_map<GCTracked *, GCTracked *, GCPtrSymHash, GCPtrSymEqual>
      variables_;
  std::shared_ptr<Scope> parent_;

public:
  using iterator = decltype(variables_.begin());
};