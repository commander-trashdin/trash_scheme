#include "gc.h"
#include "interfaces.h"
#include "scope.h"
#include "symbol.h"
#include <algorithm>
#include <ranges>

bool operator<(GCMark a, GCMark b) {
  return static_cast<int>(a) < static_cast<int>(b);
}

RetLock GCManager::Guard(GCTracked *obj) { return RetLock(obj); }

GCManager::~GCManager() {
  for (auto obj : objects_)
    delete obj;
  for (auto [_, num] : constant_numbers_)
    delete num;
  for (auto [_, sym] : constant_symbols_)
    delete sym;
  for (auto ret : return_)
    delete ret;

  delete bools_.first;
  delete bools_.second;
  delete nil_;
}

GCManager &GCManager::GetInstance() {
  static GCManager instance;
  return instance;
}

GCManager::GCManager() {
  auto wrapped_obj = new GCTracked();
  auto to_store = Boolean::AllocIn(&wrapped_obj->storage_);
  wrapped_obj->obj_ptr_ = new (to_store) Boolean(true);
  bools_.first = wrapped_obj;
  wrapped_obj = new GCTracked();
  to_store = Boolean::AllocIn(&wrapped_obj->storage_);
  wrapped_obj->obj_ptr_ = new (to_store) Boolean(false);
  bools_.first = wrapped_obj;

  nil_ = new GCTracked();
}

GCTracked *GCManager::GetBool(bool kind) {
  return kind ? bools_.first : bools_.second;
}

GCTracked *GCManager::GetNil() { return nil_; }

void GCManager::RegisterObject(GCTracked *obj) {
  objects_.insert(obj);
  currentMemoryUsage_ += sizeof(*obj);
  if (currentMemoryUsage_ >= threshold_ && phase_ != Phase::Read) {

    auto memus = currentMemoryUsage_;
    CollectGarbage();
  }
}

void GCManager::UnregisterObject(GCTracked *obj) { objects_.erase(obj); }

std::unordered_map<int64_t, GCTracked *> *GCManager::GetNumReg() {
  return &constant_numbers_;
}

std::unordered_map<std::string_view, GCTracked *> *GCManager::GetSymReg() {
  return &constant_symbols_;
}

template <NumberOrSymbol T>
GCTracked *GCManager::GetConstant(T::ValueType val) {
  auto registry = T::GetConstantRegistry();
  auto it = registry->find(val);
  if (it == registry->end())
    it = registry->emplace(val, Create<T>(val)).first;
  return it->second;
}

void GCManager::AddRoot(const std::shared_ptr<Scope> &scope) {
  roots_.insert(scope.get());
}

void GCManager::RemoveRoot(Scope *scope) { roots_.erase(scope); }

void GCManager::Sweep() {
  auto cleaner = [](auto const &obj) {
    if (obj->Color() == GCMark::White) {
      if (auto fn = Is<LambdaFunction>(obj); fn) {
        fn->Release();
      }
      delete obj;
      return true;
    }
    return false;
  };
  std::erase_if(objects_, cleaner);
}

void GCManager::MarkRoots() {
  for (const auto &scopes : roots_)
    for (auto [_, obj] : scopes->variables_)
      obj->Mark(GCMark::Grey);

  for (auto ret : return_)
    ret->Mark(GCMark::Grey);
}

void GCManager::CollectGarbage() {
  for (const auto &obj : objects_)
    obj->Mark(GCMark::White);

  MarkRoots();

  std::vector<GCTracked *> greyObjects;
  greyObjects.reserve(roots_.size() + return_.size());
  greyObjects.insert(greyObjects.end(), roots_.begin(), roots_.end());
  greyObjects.insert(greyObjects.end(), return_.begin(), return_.end());
  while (!greyObjects.empty()) {
    GCTracked *current = greyObjects.back();
    greyObjects.pop_back();
    current->Scan();
    auto newGreyObjects =
        std::ranges::subrange(current->contbegin(), current->contend()) |
        std::views::filter(
            [](GCTracked *obj) { return obj->Color() == GCMark::Grey; });

    newGreyObjects.insert(greyObjects.end(), greyObjectsView.begin(),
                          greyObjectsView.end());
  }
  Sweep();
  currentMemoryUsage_ = 0;
  for (const auto &var : objects_)
    currentMemoryUsage_ += sizeof(*var);
}

void GCManager::SetPhase(Phase phase) { phase_ = phase; }

RetLock::RetLock(GCTracked *obj) : obj_(obj) {
  GCManager::GetInstance().return_.insert(obj_);
}
RetLock::~RetLock() { GCManager::GetInstance().return_.erase(obj_); }

GCTracked::~GCTracked() {
  GCManager::GetInstance().UnregisterObject(this);
  if (obj_ptr_)
    delete obj_ptr_;
}

void GCTracked::Mark(GCMark mark) { mark_ = mark; }
void GCTracked::Scan() {
  std::for_each(obj_ptr_->contbegin(), obj_ptr_->contend(), [](GCTracked *obj) {
    if (obj->Color() == GCMark::White)
      obj->Mark(GCMark::Grey);
  });
  mark_ = GCMark::Black;
}

GCMark GCTracked::Color() const { return mark_; }

Types GCTracked::ID() const {
  if (!obj_ptr_)
    return Types::null;
  return obj_ptr_->ID();
}

bool GCTracked::IsFalse() const {
  if (!obj_ptr_)
    return true;
  return obj_ptr_->IsFalse();
}

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
