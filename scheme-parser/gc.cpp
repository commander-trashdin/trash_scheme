#include "gc.h"
#include "parser.h"
#include <vector>

void GCManager::RegisterObject(Object *obj) {
  objects_.insert(obj);
  currentMemoryUsage_ += sizeof(*obj);
  /*if (currentMemoryUsage_ >= threshold_) {
    obj->Mark();
    std::cout << "Collecting garbage!" << std::endl;
    CollectGarbage();
    obj->Unmark();
  }*/
}

void GCManager::UnregisterObject(Object *obj) {
  objects_.erase(obj);
  currentMemoryUsage_ -= sizeof(*obj);
}

void GCManager::AddRoot(const std::shared_ptr<Scope> &scope) {
  roots_.insert(scope.get());
}

void GCManager::RemoveRoot(Scope *scope) { roots_.erase(scope); }
/*
void GCManager::Sweep() {
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

void GCManager::Sweep() {
  auto temp = std::move(objects_);
  auto cleaner = [](auto const &obj) {
    if (!obj->isMarked()) {
      delete obj;
      return true;
    } else {
      obj->Unmark();
      return false;
    }
  };
  std::erase_if(temp, cleaner);
  objects_ = std::move(temp);
}

void GCManager::MarkRoots() {
  for (const auto &scopes : roots_) {
    for (auto [_, obj] : scopes->variables_) {
      obj->Mark();
    }
  }
}

void GCManager::CollectGarbage() {
  MarkRoots();
  Sweep();
  currentMemoryUsage_ = 0;
  for (const auto &var : objects_) {
    currentMemoryUsage_ += sizeof(*var);
  }
}

GCManager::~GCManager() {
  auto temp = std::move(objects_);
  objects_.clear();

  for (auto obj : temp) {
    delete obj;
  }
}

void GCManager::PrintObjectsDebug(std::ostream *out) const {
  for (const auto &obj : objects_) {
    obj->PrintDebug(out);
  }
}

void GCManager::PrintRootsDebug(std::ostream *out) const {
  for (const auto &obj : roots_) {
    for (const auto &[name, obj] : obj->variables_) {
      *out << name << " ";
      obj->PrintDebug(out);
    }
  }
  *out << std::endl;
}