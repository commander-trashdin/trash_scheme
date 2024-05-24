#pragma once

#include "gc.h"
#include "parser.h"
#include <istream>
#include <memory>
#include <sstream>
#include <string>

class SchemeInterpreter {
public:
  SchemeInterpreter();

  ~SchemeInterpreter();

  GCTracked *Eval(GCTracked *in);

  void RegisterGlobalFn(std::string name,
                        std::variant<Types, std::vector<Types>> arg_types,
                        GCTracked *(*fn)(std::shared_ptr<Scope> &,
                                         const std::vector<GCTracked *> &));

  void RegisterSF(std::string name,
                  GCTracked *(*sf)(std::shared_ptr<Scope> &,
                                   const std::vector<GCTracked *> &),
                  std::optional<size_t> arg_min = std::nullopt,
                  std::optional<size_t> arg_max = std::nullopt);

  void Load(const std::string &filename);

private:
  std::shared_ptr<Scope> global_scope_;
};
