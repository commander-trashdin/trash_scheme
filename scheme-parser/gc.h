#pragma once

#include "parser.h"
#include <algorithm>
#include <cstdint>
#include <iostream>
#include <memory>
#include <ostream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

enum class Phase { Read, Eval };

class GCManager {
public:
  static GCManager &GetInstance() {
    static GCManager instance;
    return instance;
  }

  std::unordered_set<Object *> *Guarded() { return &return_; }

  class SafeLock {
  public:
    template <typename... Args> SafeLock(Args... args) {
      (GCManager::GetInstance().Guarded()->insert(std::forward<Args>(args)),
       ...);
      (current_.push_back(std::forward<Args>(args)), ...);
    }

    void Lock(Object *obj) {
      GCManager::GetInstance().Guarded()->insert(obj);
      current_.push_back(obj);
    }

    ~SafeLock() {
      for (auto obj : current_)
        GCManager::GetInstance().Guarded()->erase(obj);
    }

  private:
    std::vector<Object *> current_;
  };

  const Boolean *GetBool(bool kind) {
    return kind ? &bools_.first : &bools_.second;
  }

  void RegisterObject(Object *obj) {
    objects_.insert(obj);
    currentMemoryUsage_ += sizeof(*obj);
    if (currentMemoryUsage_ >= threshold_ && phase_ != Phase::Read) {
      obj->Mark();
      std::cout << "Memory usage is " << currentMemoryUsage_
                << ". Collecting garbage!" << std::endl;
      auto memus = currentMemoryUsage_;
      CollectGarbage();
      obj->Unmark();
      std::cout << "Garbage collected! Freed " << memus - currentMemoryUsage_
                << " bytes of memory. New heap size is " << currentMemoryUsage_
                << std::endl;
    }
  }

  std::unordered_map<int64_t, Number *> *GetNumReg() {
    return &constant_numbers_;
  }

  std::unordered_map<std::string_view, Symbol *> *GetSymReg() {
    return &constant_symbols_;
  }

  template <NumberOrSymbol T> T *GetConstant(T::ValueType val) {
    auto registry = T::GetConstantRegistry();
    auto it = registry->find(val);
    if (it == registry->end())
      it = registry->emplace(val, new T(val)).first;
    return it->second;
  }

  /*
void  UnregisterObject(Object *obj) {
  objects_.erase(obj);
  currentMemoryUsage_ -= sizeof(*obj);
}
*/

  void AddRoot(const std::shared_ptr<Scope> &scope) {
    roots_.insert(scope.get());
  }

  void RemoveRoot(Scope *scope) { roots_.erase(scope); }
  /*
  void  Sweep() {
    for (auto it = objects_.begin(); it != objects_.end();) {
      if ((*it)->isMarked()) {
        (*it)->Unmark();
        ++it;
      } else {
        it = objects_.erase(it);
        delete *it;
      }
    }
  }
  */

  void Sweep() {
    // auto temp = std::move(objects_);
    auto cleaner = [](auto const &obj) {
      if (!obj->isMarked()) {
        if (auto fn = Is<LambdaFunction>(obj); fn) {
          fn->Release();
        }
        delete obj;
        return true;
      } else {
        obj->Unmark();
        return false;
      }
    };
    std::erase_if(objects_, cleaner);
    // objects_ = std::move(temp);
  }

  void MarkRoots() {
    for (const auto &scopes : roots_)
      for (auto [_, obj] : scopes->variables_)
        obj->Mark();

    for (auto ret : return_)
      ret->Mark();
  }

  void CollectGarbage() {
    MarkRoots();
    Sweep();
    currentMemoryUsage_ = 0;
    for (const auto &var : objects_)
      currentMemoryUsage_ += sizeof(*var);
  }

  void PrintObjectsDebug(std::ostream *out) const {
    for (const auto &obj : objects_)
      obj->PrintDebug(out);
  }

  void PrintRootsDebug(std::ostream *out) const {
    for (const auto &obj : roots_) {
      for (const auto &[name, obj] : obj->variables_) {
        *out << name << " ";
        obj->PrintDebug(out);
      }
    }
    *out << std::endl;
  }

  void SetPhase(Phase phase) { phase_ = phase; }

private:
  Phase phase_ = Phase::Read;
  std::unordered_set<Object *> objects_;
  std::unordered_set<Scope *> roots_;
  std::unordered_set<Object *> return_;
  const size_t threshold_ = 32;
  size_t currentMemoryUsage_ = 0;
  std::unordered_map<int64_t, Number *> constant_numbers_;
  const std::pair<Boolean, Boolean> bools_ = {Boolean(true), Boolean(false)};
  std::unordered_map<std::string_view, Symbol *> constant_symbols_;

  GCManager() {}
  ~GCManager() {
    // auto temp = std::move(objects_);
    // objects_.clear();

    for (auto obj : objects_)
      delete obj;
    for (auto [_, num] : constant_numbers_)
      delete num;
    for (auto [_, sym] : constant_symbols_)
      delete sym;
    for (auto ret : return_)
      delete ret;
  }
  GCManager(const GCManager &) = delete;
  GCManager &operator=(const GCManager &) = delete;
};
