#include "builtins.h"
#include "gc.h"
#include "parser.h"
#include "util.h"
#include <cstdint>
#include <fstream>
#include <numeric>

GCTracked *Quote(std::shared_ptr<Scope> &,
                 const std::vector<GCTracked *> &args) {
  return args[0];
}

GCTracked *Eval(std::shared_ptr<Scope> &scope,
                const std::vector<GCTracked *> &args) {
  auto arg = args[0];
  auto lock = GCManager::GetInstance().Guard(arg);
  if (arg->ID() == Types::symbol) {
    return scope->Lookup(arg).first;
  } else if (arg->ID() == Types::cell) {
    auto fn = arg->As<Cell>()->GetFirst();
    auto fn_args = arg->As<Cell>()->GetSecond();
    if (fn->ID() == Types::function)
      return fn->As<Function>()->Apply(scope, fn_args);
    if (fn->ID() == Types::specialform)
      return fn->As<SpecialForm>()->Apply(scope, fn_args);
    if (fn->ID() == Types::symbol) {
      auto [obj, _] = scope->Lookup(fn);
      if (obj->ID() == Types::function)
        return obj->As<Function>()->Apply(scope, fn_args);
      if (obj->ID() == Types::specialform)
        return obj->As<SpecialForm>()->Apply(scope, fn_args);
      throw RuntimeError("First argument must be a function");
    }
    throw RuntimeError("First argument must be a function");
  } else {
    return arg;
  }
}

GCTracked *If(std::shared_ptr<Scope> &scope,
              const std::vector<GCTracked *> &args) {

  auto result = Eval(scope, {args[0]});
  if (result && !result->IsFalse())
    return Eval(scope, {args[1]});
  else
    return args.size() == 2 ? Create<>() : Eval(scope, {args[2]});
}

GCTracked *And(std::shared_ptr<Scope> &scope,
               const std::vector<GCTracked *> &args) {
  for (size_t ind = 0; ind < args.size(); ++ind) {
    auto res = Eval(scope, {args[ind]});
    if (res->IsFalse())
      return Create<Boolean>(false);
    if (ind == args.size() - 1)
      return res;
  }
  return Create<Boolean>(true);
}

GCTracked *Or(std::shared_ptr<Scope> &scope,
              const std::vector<GCTracked *> &args) {
  for (size_t ind = 0; ind < args.size(); ++ind) {
    auto res = Eval(scope, {args[ind]});
    if (!res->IsFalse())
      return res;
  }
  return Create<Boolean>(false);
}

GCTracked *Define(std::shared_ptr<Scope> &scope,
                  const std::vector<GCTracked *> &args) {
  auto fst = args[0];
  if (fst->ID() == Types::symbol) {
    (*scope)[fst] = Eval(scope, {args[1]});
  } else if (fst->ID() == Types::cell) {
    auto lambda_args_cell = fst->As<Cell>()->GetSecond();
    if (lambda_args_cell->ID() != Types::cell)
      throw SyntaxError("wrong argument list");
    std::vector<GCTracked *> lambda_args;
    for (auto it = lambda_args_cell->As<Cell>()->listbegin();
         it != lambda_args_cell->As<Cell>()->listend(); ++it) {
      if ((*it)->ID() != Types::symbol)
        throw SyntaxError("wrong argument name");
      lambda_args.push_back(*it);
    }
    (*scope)[fst->As<Cell>()->GetFirst()] = Create<LambdaFunction>(
        scope, std::move(lambda_args),
        std::span<GCTracked *const>(args.begin() + 1, args.end()));
  } else {
    throw SyntaxError("wrong argument list");
  }
  return Create<>();
}

GCTracked *Set(std::shared_ptr<Scope> &scope,
               const std::vector<GCTracked *> &args) {
  auto var = args[0];
  if (var->ID() == Types::symbol) {
    auto [_, actual_scope] = scope->Lookup(var);
    std::vector<GCTracked *> v = {args[1]};
    (*actual_scope)[var] = Eval(scope, v);
  } else {
    throw RuntimeError("Trying to set something that is not a variable");
  }
  return nullptr;
}

GCTracked *Lambda(std::shared_ptr<Scope> &scope,
                  const std::vector<GCTracked *> &args) {
  if (args[0]->ID() != Types::cell)
    throw SyntaxError("Bad argument list!");

  auto lambda_args_cell = args[0]->As<Cell>();
  std::vector<GCTracked *> lambda_args;
  for (auto it = lambda_args_cell->listbegin();
       it != lambda_args_cell->listend(); ++it) {
    if ((*it)->ID() != Types::symbol)
      throw SyntaxError("wrong argument name");
    lambda_args.push_back(*it);
  }
  auto fn = Create<LambdaFunction>(
      Scope::Create(scope), std::move(lambda_args),
      std::span<GCTracked *const>(args.begin() + 1, args.end()));

  return fn;
}

GCTracked *Plus(std::shared_ptr<Scope> &,
                const std::vector<GCTracked *> &args) {

  int64_t value = std::accumulate(args.begin(), args.end(), 0,
                                  [](int64_t acc, GCTracked *arg) {
                                    return acc + arg->As<Number>()->GetValue();
                                  });
  return Create<Number>(value);
}

GCTracked *Minus(std::shared_ptr<Scope> &,
                 const std::vector<GCTracked *> &args) {
  int64_t value = args[0]->As<Number>()->GetValue();

  int64_t result = std::accumulate(args.begin() + 1, args.end(), value,
                                   [](int64_t acc, GCTracked *arg) {
                                     return acc - arg->As<Number>()->GetValue();
                                   });
  return Create<Number>(result);
}

GCTracked *Divide(std::shared_ptr<Scope> &,
                  const std::vector<GCTracked *> &args) {
  int64_t value = args[0]->As<Number>()->GetValue();
  int64_t result = std::accumulate(args.begin() + 1, args.end(), value,
                                   [](int64_t acc, GCTracked *arg) {
                                     return acc / arg->As<Number>()->GetValue();
                                   });
  return Create<Number>(result);
}

GCTracked *Multiply(std::shared_ptr<Scope> &,
                    const std::vector<GCTracked *> &args) {
  int64_t value = std::accumulate(args.begin(), args.end(), 1,
                                  [](int64_t acc, GCTracked *arg) {
                                    return acc * arg->As<Number>()->GetValue();
                                  });
  return Create<Number>(value);
}

GCTracked *CheckNull(std::shared_ptr<Scope> &,
                     const std::vector<GCTracked *> &args) {
  return Create<Boolean>(args[0] == Create<>());
}

GCTracked *CheckPair(std::shared_ptr<Scope> &,
                     const std::vector<GCTracked *> &args) {
  return Create<Boolean>(args[0]->ID() == Types::cell);
}

GCTracked *CheckNumber(std::shared_ptr<Scope> &,
                       const std::vector<GCTracked *> &args) {
  return Create<Boolean>(args[0]->ID() == Types::number);
}

GCTracked *CheckBoolean(std::shared_ptr<Scope> &,
                        const std::vector<GCTracked *> &args) {
  return Create<Boolean>(args[0]->ID() == Types::boolean);
}

GCTracked *CheckSymbol(std::shared_ptr<Scope> &,
                       const std::vector<GCTracked *> &args) {
  return Create<Boolean>(args[0]->ID() == Types::symbol);
}

GCTracked *CheckList(std::shared_ptr<Scope> &,
                     const std::vector<GCTracked *> &args) {
  for (auto fst = args[0]; fst != Create<>();
       fst = fst->As<Cell>()->GetSecond()) {
    if (fst->ID() != Types::cell)
      return Create<Boolean>(false);
  }
  return Create<Boolean>(true);
}

GCTracked *Eq(std::shared_ptr<Scope> &, const std::vector<GCTracked *> &args) {
  return Create<Boolean>(args[0] == args[1]);
}

GCTracked *Eql(std::shared_ptr<Scope> &, const std::vector<GCTracked *> &args) {
  auto fst = args[0];
  auto snd = args[1];
  return Create<Boolean>(fst->ID() == snd->ID() && *fst == *snd);
}

GCTracked *Not(std::shared_ptr<Scope> &, const std::vector<GCTracked *> &args) {
  return Create<Boolean>(args[0] == Create<>() ? false : args[0]->IsFalse());
}

GCTracked *More(std::shared_ptr<Scope> &,
                const std::vector<GCTracked *> &args) {
  for (size_t i = 1; i < args.size(); ++i) {
    if (args[i - 1]->As<Number>()->GetValue() <=
        args[i]->As<Number>()->GetValue())
      return Create<Boolean>(false);
  }
  return Create<Boolean>(true);
}

GCTracked *Less(std::shared_ptr<Scope> &,
                const std::vector<GCTracked *> &args) {
  for (size_t i = 1; i < args.size(); ++i) {
    if (args[i - 1]->As<Number>()->GetValue() >=
        args[i]->As<Number>()->GetValue())
      return Create<Boolean>(false);
  }
  return Create<Boolean>(true);
}

GCTracked *MoreOrEqual(std::shared_ptr<Scope> &,
                       const std::vector<GCTracked *> &args) {
  for (size_t i = 1; i < args.size(); ++i) {
    if (args[i - 1]->As<Number>()->GetValue() <
        args[i]->As<Number>()->GetValue())
      return Create<Boolean>(false);
  }
  return Create<Boolean>(true);
}

GCTracked *LessOrEqual(std::shared_ptr<Scope> &,
                       const std::vector<GCTracked *> &args) {
  for (size_t i = 1; i < args.size(); ++i) {
    if (args[i - 1]->As<Number>()->GetValue() >
        args[i]->As<Number>()->GetValue())
      return Create<Boolean>(false);
  }
  return Create<Boolean>(true);
}

GCTracked *Min(std::shared_ptr<Scope> &, const std::vector<GCTracked *> &args) {
  int64_t value = args[0]->As<Number>()->GetValue();
  for (const auto &obj : args) {
    int64_t current = obj->As<Number>()->GetValue();
    if (value > current)
      value = current;
  }
  return Create<Number>(value);
}

GCTracked *Max(std::shared_ptr<Scope> &, const std::vector<GCTracked *> &args) {
  int64_t value = args[0]->As<Number>()->GetValue();
  for (const auto &obj : args) {
    int64_t current = obj->As<Number>()->GetValue();
    if (value < current)
      value = current;
  }
  return Create<Number>(value);
}

GCTracked *Cons(std::shared_ptr<Scope> &,
                const std::vector<GCTracked *> &args) {
  return Create<Cell>(args[0], args[1]);
}

GCTracked *Car(std::shared_ptr<Scope> &, const std::vector<GCTracked *> &args) {
  return args[0]->As<Cell>()->GetFirst();
}

GCTracked *Cdr(std::shared_ptr<Scope> &, const std::vector<GCTracked *> &args) {
  return args[0]->As<Cell>()->GetSecond();
}

GCTracked *SetCar(std::shared_ptr<Scope> &,
                  const std::vector<GCTracked *> &args) {
  args[0]->As<Cell>()->SetFirst(args[1]);
  return args[0];
}

GCTracked *SetCdr(std::shared_ptr<Scope> &,
                  const std::vector<GCTracked *> &args) {
  args[0]->As<Cell>()->SetSecond(args[1]);
  return args[0];
}

GCTracked *List(std::shared_ptr<Scope> &,
                const std::vector<GCTracked *> &args) {
  if (args.size() == 0)
    return nullptr;

  auto new_cell = Create<Cell>();
  auto res = new_cell;
  auto lock = GCManager::GetInstance().Guard(res);
  for (size_t ind = 0; ind < args.size();
       ++ind, new_cell = new_cell->As<Cell>()->GetSecond()) {
    new_cell->As<Cell>()->SetFirst(args[ind]);
    if (ind != args.size() - 1)
      new_cell->As<Cell>()->SetSecond(Create<Cell>());
  }
  return res;
}

GCTracked *ListRef(std::shared_ptr<Scope> &,
                   const std::vector<GCTracked *> &args) {
  auto scope = args[0];
  auto value = args[1]->As<Number>()->GetValue();
  while (value != 0) {
    if (scope->ID() != Types::cell)
      throw RuntimeError("List is too short");
    scope = scope->As<Cell>()->GetSecond();
    --value;
  }
  if (scope->ID() != Types::cell)
    throw RuntimeError("List is too short");

  return scope->As<Cell>()->GetFirst();
}

GCTracked *ListTail(std::shared_ptr<Scope> &,
                    const std::vector<GCTracked *> &args) {
  auto scope = args[0];
  while (true) {
    if (scope->ID() != Types::cell)
      throw RuntimeError("Not a proper list");
    auto cell = scope->As<Cell>();
    if (cell->GetSecond()->ID() == Types::null)
      return scope;
    else
      scope = cell->GetSecond();
  }
}

GCTracked *Map(std::shared_ptr<Scope> &, const std::vector<GCTracked *> &args) {
  auto new_cell = Create<Cell>();
  auto res = new_cell;
  auto lock = GCManager::GetInstance().Guard(res);
  auto source = args[1]->As<Cell>();
  auto fn = args[0]->As<Function>();
  while (true) {
    if (source->GetSecond()->ID() != Types::cell)
      throw RuntimeError("Syntax error!");
    std::shared_ptr<Scope> nullscope = nullptr;
    auto result = fn->Apply(nullscope, source->GetFirst());
    new_cell->As<Cell>()->SetFirst(result);
    if (source->GetSecond()->ID() == Types::cell) {
      source = source->GetSecond()->As<Cell>();
      new_cell->As<Cell>()->SetSecond(Create<Cell>());
      new_cell = new_cell->As<Cell>()->GetSecond();
    } else if (source->GetSecond()->ID() == Types::null) {
      break;
    } else {
      throw RuntimeError("Syntax error!");
    }
  }
  return res;
}

GCTracked *Exit(std::shared_ptr<Scope> &,
                const std::vector<GCTracked *> &args) {
  return Create<BuiltInObject>();
}

GCTracked *Load(std::shared_ptr<Scope> &scope,
                const std::vector<GCTracked *> &args) {
  auto filename = args[0]->As<String>()->GetValue();
  if (!hasCorrectExtension(filename)) {
    throw RuntimeError("Wrong file extension");
  }
  std::ifstream filestream(filename);
  Parser parser((Tokenizer(&filestream)));
  auto global_scope = scope->GetGlobalScope();
  GCManager::GetInstance().SetPhase(Phase::Read);
  while (auto obj = parser.Read()) {
    GCManager::GetInstance().SetPhase(Phase::Eval);
    Eval(global_scope, {obj});
    GCManager::GetInstance().SetPhase(Phase::Read);
  }
  return Create<>();
}

GCTracked *Print(std::shared_ptr<Scope> &,
                 const std::vector<GCTracked *> &args) {
  args[0]->PrintTo(&std::cout);
  std::cout << std::endl;
  return args[0];
}

GCTracked *Read(std::shared_ptr<Scope> &scope,
                const std::vector<GCTracked *> &args) {
  Parser parser((Tokenizer(&std::cin)));
  GCManager::GetInstance().SetPhase(Phase::Read);
  auto obj = parser.Read();
  if (obj == nullptr)
    throw RuntimeError("Read error");
  GCManager::GetInstance().SetPhase(Phase::Eval);
  return obj;
}