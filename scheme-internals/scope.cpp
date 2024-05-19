#pragma once
#include "scope.h"
#include "gc.h"
#include "interfaces.h"
#include "util.h"

Scope::~Scope() {}

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

std::size_t GCPtrSymHash::operator()(GCTracked *k) const {
  if (k->ID() != Types::symbol)
    throw std::runtime_error("GCPtrSymHash: not a symbol");
  return std::hash<std::string>()(k->As<Symbol>()->GetName());
}

bool GCPtrSymEqual::operator()(GCTracked *lhs, GCTracked *rhs) const {
  if (lhs->ID() != Types::symbol || rhs->ID() != Types::symbol)
    throw std::runtime_error("GCPtrSymEqual: not a symbol");
  return lhs->As<Symbol>()->GetName() == rhs->As<Symbol>()->GetName();
}
