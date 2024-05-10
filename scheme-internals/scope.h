#include "interfaces.h"

class Scope : public std::enable_shared_from_this<Scope> {
public:
  static std::shared_ptr<Scope> Create();

  static std::shared_ptr<Scope> Create(std::shared_ptr<Scope> &parent);

  Scope() = default;
  explicit Scope(std::shared_ptr<Scope> &parent) : parent_(parent){};
  ~Scope();
  void Release();

  std::pair<Object *, std::shared_ptr<Scope>> Lookup(const std::string &name);

  Object *&operator[](Symbol *symbol);

  std::unordered_map<std::string, Object *> variables_;
  std::shared_ptr<Scope> parent_;
};