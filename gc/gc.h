#pragma once

#include <vector>
#ifndef MY_TEMPLATE_H
#define MY_TEMPLATE_H

#include "interfaces.h"
#include "number.h"
#include "storage.h"
#include <functional>
#include <memory>
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>

enum class Phase { Read, Eval };

enum class GCMark : int { White = 0, Grey = 1, Black = 2, Constant = 3 };

bool operator<(GCMark a, GCMark b);

class RetLock {
  friend class GCManager;

public:
  ~RetLock();
  void Lock(GCTracked *obj);

private:
  RetLock(GCTracked *obj);

  std::vector<GCTracked *> obj_;
};

GCTracked *GetBool(bool kind);
GCTracked *GetNil();

template <DerivedFromObject Derived>
GCTracked *GetConstant(typename Derived::ValueType &val);

void RegisterObject(GCTracked *obj);

template <DerivedFromObject Derived = void, ConstTag Tag = void,
          typename... Args>
  requires Creatable<Derived, Tag>
GCTracked *Create(Args &&...args);

class GCTracked {
public:
  friend class GCManager;

  explicit GCTracked(GCMark mark = GCMark::White);

  void Mark(GCMark mark);
  void Scan();

  void Walk(const std::function<void(GCTracked *)> &walker);
  [[nodiscard]] GCMark Color() const;

  template <DerivedFromObject Derived> Derived *As() {
    return static_cast<Derived *>(obj_ptr_);
  }

  void PrintTo(std::ostream *out) const;

  [[nodiscard]] std::string Serialize() const;

  [[nodiscard]] Types ID() const;

  [[nodiscard]] bool IsFalse() const;

  bool operator==(const GCTracked &other) const;

  template <DerivedFromObject Derived, ConstTag Tag, typename... Args>
    requires Creatable<Derived, Tag>
  friend GCTracked *Create(Args &&...args) {
    if constexpr (std::is_same_v<Derived, void>) {
      return GetNil();
    } else if constexpr (std::is_same_v<Derived, Boolean>) {
      return GetBool(std::forward<Args>(args)...);
    } else if constexpr (std::is_same_v<Tag, constant>) {
      return GetConstant<Derived>(std::forward<Args>(args)...);
    } else {
      auto wrapped_obj = new GCTracked();
      auto to_store = Derived::AllocIn(&wrapped_obj->storage_);
      wrapped_obj->obj_ptr_ =
          new (to_store) Derived(std::forward<Args>(args)...);
      RegisterObject(wrapped_obj);
      return wrapped_obj;
    }
  }

  ~GCTracked();
  template <DerivedFromObject Derived>
  friend GCTracked *GetConstant(typename Derived::ValueType &val);

private:
  [[nodiscard]] Object *Get() const;
  T storage_;
  Object *obj_ptr_ = nullptr;
  GCMark mark_;
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

  std::unordered_map<std::string, GCTracked *> *GetSymReg();

  std::unordered_map<std::string, GCTracked *> *GetErrorReg();

  template <DerivedFromObject Dervied>
  std::unordered_map<typename Dervied::ValueType, GCTracked *> *
  GetConstantRegistry() {
    if constexpr (std::is_same_v<Dervied, Number>)
      return &constant_numbers_;
    else if constexpr (std::is_same_v<Dervied, Symbol>)
      return &constant_symbols_;
    else if constexpr (std::is_same_v<Dervied, RuntimeError>)
      return &errors_;
    else
      static_assert(sizeof(Dervied) == 0, "Invalid type");
    return nullptr;
  }

  void AddRoot(const std::shared_ptr<Scope> &scope);

  void Sweep();

  void MarkRoots();

  void CollectGarbage();

  void PrintObjectsDebug(std::ostream *out) const;

  void PrintRootsDebug(std::ostream *out) const;

  void SetPhase(Phase phase);

  GCManager(const GCManager &) = delete;
  GCManager &operator=(const GCManager &) = delete;

private:
  Phase phase_ = Phase::Read;
  std::unordered_set<GCTracked *> objects_;
  std::unordered_set<std::shared_ptr<Scope>> roots_;
  std::unordered_set<GCTracked *> return_;
  const size_t threshold_ = 32;
  size_t currentMemoryUsage_ = 0;
  std::unordered_map<int64_t, GCTracked *> constant_numbers_;
  std::unordered_map<std::string, GCTracked *> constant_symbols_;
  std::unordered_map<std::string, GCTracked *> errors_;
  std::pair<GCTracked *, GCTracked *> bools_;
  GCTracked *nil_;
  GCManager();
  ~GCManager();
};

#include "getconst.h"
#endif // MY_TEMPLATE_H