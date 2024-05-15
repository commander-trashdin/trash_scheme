#include "scope.h"
#include "gc.h"
#include "interfaces.h"
#include "util.h"

Scope::~Scope() { GCManager::GetInstance().RemoveRoot(this); }

std::shared_ptr<Scope> Scope::Create() {
  std::shared_ptr<Scope> newScope = std::make_shared<Scope>();
  GCManager::GetInstance().AddRoot(newScope);
  return newScope;
}

std::shared_ptr<Scope> Scope::Create(std::shared_ptr<Scope> &parent) {
  std::shared_ptr<Scope> newScope = std::make_shared<Scope>(parent);
  GCManager::GetInstance().AddRoot(newScope);
  return newScope;
}

std::pair<GCTracked *, std::shared_ptr<Scope>> Scope::Lookup(GCTracked *sym) {
  auto it = variables_.find(sym);
  if (it == variables_.end()) {
    if (!parent_)
      throw NameError(sym->As<Symbol>()->GetName());
    else
      return parent_->Lookup(sym);
  }
  return {it->second, shared_from_this()};
}

GCTracked *&Scope::operator[](GCTracked *sym) { return variables_[sym]; }
