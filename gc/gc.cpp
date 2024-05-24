#include "gc.h"
#include "interfaces.h"
#include "number.h"
#include "scope.h"
#include <algorithm>
#include <fstream>
#include <functional>
#include <iostream>
#include <ostream>
#include <ranges>
#include <sstream>
#include <string>
#include <unordered_set>
#include <vector>

bool operator<(GCMark a, GCMark b) {
  return static_cast<int>(a) < static_cast<int>(b);
}

RetLock GCManager::Guard(GCTracked *obj) { return RetLock(obj); }

GCTracked *GetBool(bool kind) { return GCManager::GetInstance().GetBool(kind); }
GCTracked *GetNil() { return GCManager::GetInstance().GetNil(); }

void RegisterObject(GCTracked *obj) {
  GCManager::GetInstance().RegisterObject(obj);
}

GCManager::~GCManager() {
  for (auto obj : objects_)
    delete obj;
  for (auto [_, num] : constant_numbers_)
    delete num;
  for (auto [_, sym] : constant_symbols_)
    delete sym;

  delete bools_.first;
  delete bools_.second;
  delete nil_;
  roots_.clear();
}

GCManager &GCManager::GetInstance() {
  static GCManager instance;
  return instance;
}

GCManager::GCManager() {
  auto wrapped_obj = new GCTracked(GCMark::Constant);
  auto to_store = Boolean::AllocIn(&wrapped_obj->storage_);
  wrapped_obj->obj_ptr_ = new (to_store) Boolean(true);
  bools_.first = wrapped_obj;
  wrapped_obj = new GCTracked(GCMark::Constant);
  to_store = Boolean::AllocIn(&wrapped_obj->storage_);
  wrapped_obj->obj_ptr_ = new (to_store) Boolean(false);
  bools_.second = wrapped_obj;

  nil_ = new GCTracked(GCMark::Constant);
}

GCTracked *GCManager::GetBool(bool kind) {
  return kind ? bools_.first : bools_.second;
}

GCTracked *GCManager::GetNil() { return nil_; }

void GCManager::RegisterObject(GCTracked *obj) {
  objects_.insert(obj);
  return_.insert(obj);
  currentMemoryUsage_ += sizeof(*obj);
  if (currentMemoryUsage_ >= threshold_ && phase_ != Phase::Read)
    CollectGarbage();
  return_.erase(obj);
}

void GCManager::UnregisterObject(GCTracked *obj) {
  objects_.erase(obj);
  return_.erase(obj);
}

std::unordered_map<int64_t, GCTracked *> *GCManager::GetNumReg() {
  return &constant_numbers_;
}

std::unordered_map<std::string, GCTracked *> *GCManager::GetSymReg() {
  return &constant_symbols_;
}

std::unordered_map<std::string, GCTracked *> *GCManager::GetErrorReg() {
  return &errors_;
}

void GCManager::AddRoot(const std::shared_ptr<Scope> &scope) {
  roots_.insert(scope);
}

void GCManager::Sweep() {
  auto cleaner = [](auto const &obj) {
    if (obj->Color() == GCMark::White) {
      delete obj;
      return true;
    }
    return false;
  };
  std::erase_if(objects_, cleaner);
}

void GCManager::MarkRoots() {
  for (const auto &scopes : roots_)
    for (auto [key, obj] : scopes->variables_) {
      key->Mark(GCMark::Grey);
      obj->Mark(GCMark::Grey);
    }

  for (auto ret : return_)
    ret->Mark(GCMark::Grey);
}

void GCManager::CollectGarbage() {
  for (const auto &obj : objects_)
    obj->Mark(GCMark::White);

  MarkRoots();

  std::vector<GCTracked *> greyObjects;
  std::unordered_set<GCTracked *> removed;
  greyObjects.reserve(roots_.size() + return_.size());
  for (auto &root : roots_)
    for (auto [var, obj] : root->variables_) {
      greyObjects.push_back(obj);
      greyObjects.push_back(var);
    }
  greyObjects.insert(greyObjects.end(), return_.begin(), return_.end());

  while (!greyObjects.empty()) {
    GCTracked *current = greyObjects.back();
    greyObjects.pop_back();
    removed.insert(current);
    current->Scan();
    current->Get()->Walk([&greyObjects, &removed](GCTracked *obj) {
      if (obj->Color() == GCMark::Grey && !removed.contains(obj)) {
        greyObjects.push_back(obj);
      }
    });
  }
  Sweep();
  std::erase_if(roots_, [](auto const &obj) { return obj.use_count() == 1; });
  currentMemoryUsage_ = 0;
  for (const auto &var : objects_)
    currentMemoryUsage_ += sizeof(*var);
}

void GCManager::SetPhase(Phase phase) { phase_ = phase; }

RetLock::RetLock(GCTracked *obj) : obj_({obj}) {
  GCManager::GetInstance().return_.insert(obj);
}

void RetLock::Lock(GCTracked *obj) {
  obj_.push_back(obj);
  GCManager::GetInstance().return_.insert(obj);
}
RetLock::~RetLock() {
  for (auto obj : obj_)
    GCManager::GetInstance().return_.erase(obj);
}

GCTracked::GCTracked(GCMark mark) : mark_(mark) {}

GCTracked::~GCTracked() {
  if (obj_ptr_)
    obj_ptr_->~Object();
}

void GCTracked::Mark(GCMark mark) {
  if (mark_ != GCMark::Constant)
    mark_ = mark;
}

void GCTracked::Walk(const std::function<void(GCTracked *)> &walker) {
  walker(this);
  if (obj_ptr_)
    obj_ptr_->Walk(walker);
}

void GCTracked::Scan() {
  Walk([](GCTracked *obj) {
    if (obj->Color() == GCMark::White) {
      obj->Mark(GCMark::Grey);
    }
  });
  mark_ = GCMark::Black;
}

GCMark GCTracked::Color() const { return mark_; }

Types GCTracked::ID() const { return obj_ptr_ ? obj_ptr_->ID() : Types::null; }

bool GCTracked::IsFalse() const { return !obj_ptr_ || obj_ptr_->IsFalse(); }

bool GCTracked::operator==(const GCTracked &other) const {
  if (!obj_ptr_ && !other.obj_ptr_)
    return true;
  if (!obj_ptr_ || !other.obj_ptr_)
    return false;
  return *obj_ptr_ == *other.obj_ptr_;
}

void GCTracked::PrintTo(std::ostream *out) const {
  if (!obj_ptr_)
    *out << "()";
  else
    obj_ptr_->PrintTo(out);
}

std::string GCTracked::Serialize() const {
  if (!obj_ptr_)
    return "NIL";
  std::stringstream ss;
  obj_ptr_->PrintTo(&ss);
  return ss.str();
}

Object *GCTracked::Get() const { return obj_ptr_; }
