#pragma once
#ifndef MY_TEMPLATE_IMPL_H
#define MY_TEMPLATE_IMPL_H

#include "gc.h"

template <DerivedFromObject Derived>
GCTracked *GetConstant(typename Derived::ValueType &val) {
  auto registry = GCManager::GetInstance().GetConstantRegistry<Derived>();
  auto it = registry->find(val);
  if (it == registry->end()) {
    auto wrapped_obj = new GCTracked(GCMark::Constant);
    auto to_store = Derived::AllocIn(&wrapped_obj->storage_);
    wrapped_obj->obj_ptr_ =
        new (to_store) Derived(typename Derived::ValueType(val));
    it = registry->emplace(val, wrapped_obj).first;
  }
  return it->second;
}

#endif // MY_TEMPLATE_IMPL_H