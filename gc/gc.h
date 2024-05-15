#pragma once
#include "interfaces.h"
#include "storage.h"
#include <unordered_map>
#include <unordered_set>

enum class Phase { Read, Eval };

enum class GCMark : int { White = 0, Grey = 1, Black = 2 };

bool operator<(GCMark a, GCMark b);

class RetLock {
  friend class GCManager;

public:
  ~RetLock();

private:
  RetLock(GCTracked *obj);

  GCTracked *obj_;
};

class GCManager {
public:
  static GCManager &GetInstance();
  friend class RetLock;
  RetLock Guard(GCTracked *obj);

  GCTracked *GetBool(bool kind);
  GCTracked *GetNil();

  void RegisterObject(GCTracked *obj);
  void UnregisterObject(GCTracked *obj);

  std::unordered_map<int64_t, GCTracked *> *GetNumReg();

  std::unordered_map<std::string_view, GCTracked *> *GetSymReg();

  template <NumberOrSymbol T> GCTracked *GetConstant(T::ValueType val);

  void AddRoot(const std::shared_ptr<Scope> &scope);

  void RemoveRoot(Scope *scope);
  void Sweep();

  void MarkRoots();

  void CollectGarbage();

  void PrintObjectsDebug(std::ostream *out) const;

  void PrintRootsDebug(std::ostream *out) const;

  void SetPhase(Phase phase);

private:
  Phase phase_ = Phase::Read;
  std::unordered_set<GCTracked *> objects_;
  std::unordered_set<Scope *> roots_;
  std::unordered_set<GCTracked *> return_;
  const size_t threshold_ = 32;
  size_t currentMemoryUsage_ = 0;
  std::unordered_map<int64_t, GCTracked *> constant_numbers_;
  std::unordered_map<std::string_view, GCTracked *> constant_symbols_;
  std::pair<GCTracked *, GCTracked *> bools_;
  GCTracked *nil_;

  GCManager();
  ~GCManager();
  GCManager(const GCManager &) = delete;
  GCManager &operator=(const GCManager &) = delete;
};

template <DerivedFromObject Derived = void, ConstTag Tag = void,
          typename... Args>
  requires Creatable<Derived, Tag>
GCTracked *Create(Args &&...args);

class GCTracked {
public:
  friend class GCManager;

  GCTracked() = default;

  void Mark(GCMark mark);
  void Scan();

  GCMark Color() const;

  template <DerivedFromObject Derived> Derived *As() {
    return static_cast<Derived *>(obj_ptr_);
  }

  void PrintTo(std::ostream *out) const;

  Types ID() const;

  bool IsFalse() const;

  bool operator==(const GCTracked &other) const;

  template <DerivedFromObject Derived, ConstTag Tag, typename... Args>
    requires Creatable<Derived, Tag>
  friend GCTracked *Create(Args &&...args) {
    if constexpr (is_static_v<Derived>) {
      if constexpr (std::is_same_v<Derived, void>)
        return GCManager::GetInstance().GetNil();
      return GCManager::GetInstance().GetBool(std::forward<Args>(args)...);
    } else if constexpr (std::is_same_v<Tag, constant>) {
      return GCManager::GetInstance().GetConstant<Derived>(
          std::forward<Args>(args)...);
    } else {
      auto wrapped_obj = new GCTracked();
      auto to_store = Derived::AllocIn(&wrapped_obj->storage_);
      wrapped_obj->obj_ptr_ =
          new (to_store) Derived(std::forward<Args>(args)...);
      GCManager::GetInstance().RegisterObject(wrapped_obj);
      return wrapped_obj;
    }
  }

  ~GCTracked();

private:
  T storage_;
  Object *obj_ptr_ = nullptr;
  GCMark mark_ = GCMark::White;
};

struct GCPtrSymHash {
  std::size_t operator()(GCTracked *k) const;
};

struct GCPtrSymEqual {
  bool operator()(GCTracked *lhs, GCTracked *rhs) const;
};
