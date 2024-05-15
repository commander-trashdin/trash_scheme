#pragma once

#include "gc.h"
#include "parser.h"
#include <istream>
#include <memory>
#include <sstream>

class SchemeInterpreter {
public:
  SchemeInterpreter();

  ~SchemeInterpreter();

  GCTracked *Eval(Object *in);

  void RegisterGlobalFn(const std::string &name,
                        std::variant<Types, std::vector<Types>> arg_types,
                        GCTracked *(*fn)(std::shared_ptr<Scope> &,
                                         const std::vector<GCTracked *> &));

  void RegisterSF(const std::string &name,
                  GCTracked *(*sf)(std::shared_ptr<Scope> &,
                                   const std::vector<GCTracked *> &),
                  std::optional<size_t> arg_min = std::nullopt,
                  std::optional<size_t> arg_max = std::nullopt);

  void REPL(std::istream *in = &std::cin);

private:
  std::shared_ptr<Scope> global_scope_;
};
