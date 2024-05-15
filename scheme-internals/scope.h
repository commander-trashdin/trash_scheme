#include "gc.h"
#include "interfaces.h"
#include "symbol.h"

class Scope : public std::enable_shared_from_this<Scope> {
public:
  static std::shared_ptr<Scope> Create();

  static std::shared_ptr<Scope> Create(std::shared_ptr<Scope> &parent);

  Scope() = default;
  explicit Scope(std::shared_ptr<Scope> &parent) : parent_(parent) {};
  ~Scope();

  std::pair<GCTracked *, std::shared_ptr<Scope>> Lookup(GCTracked *sym);

  GCTracked *&operator[](GCTracked *symbol);

  std::unordered_map<GCTracked *, GCTracked *, GCPtrSymHash, GCPtrSymEqual>
      variables_;
  std::shared_ptr<Scope> parent_;
};