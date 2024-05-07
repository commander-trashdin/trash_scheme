#include "gc.h"
#include "parser.h"
#include <utility>

template <DerivedFromObject Derived, ConstTag Tag = void, typename... Args>
  requires Creatable<Derived, Tag>
Derived *Create(Args &&...args) {
  if constexpr (std::is_same_v<Tag, void>) {
    auto obj = new Derived(std::forward<Args>(args)...);
    GCManager::GetInstance().RegisterObject(obj);
    return obj;
  } else
    return GCManager::GetInstance().GetConstant<Derived>(
        std::forward<Args>(args)...);
}

template <ConstTag Tag, typename... Args> Boolean *Create(Args &&...args) {
  return const_cast<Boolean *>(
      GCManager::GetInstance().GetBool(std::forward<Args>(args)...));
}
