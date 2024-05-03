#pragma once

#include <algorithm>
#include <iostream>
#include <memory>
#include <unordered_set>
class Object;
class Scope;

class GCManager {
public:
  static GCManager &GetInstance() {
    static GCManager instance;
    return instance;
  }

  void RegisterObject(Object *obj);

  void UnregisterObject(Object *obj);

  void AddRoot(const std::shared_ptr<Scope> &scope);

  void RemoveRoot(Scope *scope);

  void Sweep();

  void MarkRoots();

  void CollectGarbage();

  void PrintObjectsDebug(std::ostream *out = &std::cout) const;

  void PrintRootsDebug(std::ostream *out = &std::cout) const;

private:
  std::unordered_set<Object *> objects_;
  std::unordered_set<Scope *> roots_;
  const size_t threshold_ = 32;
  size_t currentMemoryUsage_ = 0;

  GCManager() {}
  ~GCManager();
  GCManager(const GCManager &) = delete;
  GCManager &operator=(const GCManager &) = delete;
};
