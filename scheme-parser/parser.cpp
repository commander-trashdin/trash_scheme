#include <parser.h>
#include <iostream>

Scope::Scope() : parent_(nullptr) {
}

Scope::Scope(std::shared_ptr<Scope>& parent) : parent_(parent) {
}

std::shared_ptr<Object> Scope::Lookup(const std::string& name) {
    auto it = variables_.find(name);
    if (it == variables_.end()) {
        if (!parent_) {
            throw NameError(name);
        } else {
            return parent_->Lookup(name);
        }
    }
    return it->second;
}

Types Object::ID() const {
    return Types::tType;
}

bool Object::IsFalse() {
    return false;
}

Object::~Object() = default;

Cell::Cell() : head_(nullptr), tail_(nullptr) {
}

Cell::Cell(std::shared_ptr<Object> head, std::shared_ptr<Object> tail) : head_(head), tail_(tail) {
}

Types Cell::ID() const {
    return Types::cellType;
}

void Cell::PrintTo(std::ostream* out) {
    *out << '(';
    ::PrintTo(head_, out);
    auto next = AsCell(tail_);
    if (!next && tail_) {
        *out << " . ";
        ::PrintTo(tail_, out);
    } else {
        while (next) {
            *out << " ";
            ::PrintTo(next->head_, out);
            auto next_obj = AsCell(next->tail_);
            if (!next_obj && next->tail_) {
                *out << " . ";
                ::PrintTo(next->tail_, out);
            }
            next = next_obj;
        }
    }
    *out << ')';
    return;
}

std::shared_ptr<Object> Cell::Eval(const std::shared_ptr<Scope>& scope) {
    if (head_ == nullptr) {
        throw RuntimeError("First element of the list is not a function");
    }
    auto ptr = head_->Eval(scope);
    auto fn = std::dynamic_pointer_cast<Function>(ptr);
    auto sf = std::dynamic_pointer_cast<SpecialForm>(ptr);
    if (!fn && !sf) {
        throw RuntimeError("First element of the list must be a function");
    }
    std::vector<std::shared_ptr<Object>> args = ToVector(tail_);
    if (fn) {
        for (auto& arg : args) {
            arg = arg->Eval(scope);
        }
    }
    if (fn) {
        return fn->Apply(scope, args);
    } else {
        return sf->Apply(scope, args);
    }
}

const std::shared_ptr<Object>& Cell::GetFirst() const {
    return head_;
}

void Cell::SetFirst(std::shared_ptr<Object> object) {
    head_ = object;
}

const std::shared_ptr<Object>& Cell::GetSecond() const {
    return tail_;
}

void Cell::SetSecond(std::shared_ptr<Object> object) {
    tail_ = object;
}

Number::Number() : value_(0) {
}

Number::Number(int64_t value) : value_(value) {
}

Types Number::ID() const {
    return Types::numberType;
}

void Number::PrintTo(std::ostream* out) {
    *out << value_;
}

std::shared_ptr<Object> Number::Eval(const std::shared_ptr<Scope>&) {
    return shared_from_this();
}

int64_t Number::GetValue() const {
    return value_;
}

int64_t& Number::SetValue() {
    return value_;
}

Symbol::Symbol() : name_("") {
}

Symbol::Symbol(std::string name) : name_(name) {
}

Types Symbol::ID() const {
    return Types::symbolType;
}

void Symbol::PrintTo(std::ostream* out) {
    *out << name_;
}

std::shared_ptr<Object> Symbol::Eval(const std::shared_ptr<Scope>& scope) {
    return scope->Lookup(name_);
}

const std::string& Symbol::GetName() const {
    return name_;
}

std::string& Symbol::SetName() {
    return name_;
}

Types Quote::ID() const {
    return Types::quoteType;
}

std::shared_ptr<Object> Quote::Apply(const std::shared_ptr<Scope>& scope,
                                     const std::vector<std::shared_ptr<Object>>& args) {
    (void)scope;
    if (args.size() != 1) {
        throw SyntaxError("Syntax error!");  // FIXME
    }
    return args[0];
}

Boolean::Boolean() {
}

Boolean::Boolean(bool val) {
    if (val) {
        name_ = "#t";
    } else {
        name_ = "#f";
    }
}

std::shared_ptr<Object> Boolean::Eval(const std::shared_ptr<Scope>&) {
    return shared_from_this();
}

bool Boolean::IsFalse() {
    if (this->GetName() == "#f") {
        return true;
    }
    return false;
}

Types Dot::ID() const {
    return Types::dotType;
}

void Function::PrintTo(std::ostream*) {
    throw RuntimeError("can't print function");
}

void SpecialForm::PrintTo(std::ostream*) {
    throw RuntimeError("can't print special form");
}

std::shared_ptr<Object> Function::Eval(const std::shared_ptr<Scope>&) {
    throw RuntimeError("can't eval function");
}

std::shared_ptr<Object> SpecialForm::Eval(const std::shared_ptr<Scope>&) {
    throw RuntimeError("can't eval function");
}

std::shared_ptr<Object> Plus::Apply(const std::shared_ptr<Scope>&,
                                    const std::vector<std::shared_ptr<Object>>& args) {
    int64_t value = 0;
    for (const auto& arg : args) {
        auto number = std::dynamic_pointer_cast<Number>(arg);
        if (!number) {
            throw RuntimeError("+ arguments must be numbers");
        }
        value += number->GetValue();
    }
    return std::make_shared<Number>(value);
}

std::shared_ptr<Object> Minus::Apply(const std::shared_ptr<Scope>&,
                                     const std::vector<std::shared_ptr<Object>>& args) {
    if (args.size() == 0) {
        throw RuntimeError("Wrongs number of arguments!");
    }
    int64_t value = std::dynamic_pointer_cast<Number>(args[0])->GetValue();
    if (!value) {
        throw RuntimeError("- arguments must be numbers");
    }
    for (size_t i = 1; i < args.size(); ++i) {
        auto number = std::dynamic_pointer_cast<Number>(args[i]);
        if (!number) {
            throw RuntimeError("- arguments must be numbers");
        }
        value -= number->GetValue();
    }
    return std::make_shared<Number>(value);
}

std::shared_ptr<Object> Divide::Apply(const std::shared_ptr<Scope>&,
                                      const std::vector<std::shared_ptr<Object>>& args) {
    if (args.size() == 0) {
        throw RuntimeError("Wrongs number of arguments!");
    }
    int64_t value = std::dynamic_pointer_cast<Number>(args[0])->GetValue();
    if (!value) {
        throw RuntimeError("/ arguments must be numbers");
    }
    for (size_t i = 1; i < args.size(); ++i) {
        auto number = std::dynamic_pointer_cast<Number>(args[i]);
        if (!number) {
            throw RuntimeError("/ arguments must be numbers");
        }
        value /= number->GetValue();
    }
    return std::make_shared<Number>(value);
}

std::shared_ptr<Object> Multiply::Apply(const std::shared_ptr<Scope>&,
                                        const std::vector<std::shared_ptr<Object>>& args) {
    int64_t value = 1;
    for (const auto& arg : args) {
        auto number = std::dynamic_pointer_cast<Number>(arg);
        if (!number) {
            throw RuntimeError("* arguments must be numbers");
        }
        value *= number->GetValue();
    }
    return std::make_shared<Number>(value);
}

std::shared_ptr<Object> If::Apply(const std::shared_ptr<Scope>& scope,
                                  const std::vector<std::shared_ptr<Object>>& args) {
    if (args.size() < 2 || args.size() > 3) {
        throw SyntaxError("Syntax error!");
    }
    auto condition = args[0];
    auto if_true = args[1];
    auto result = condition->Eval(scope);
    if (result && !result->IsFalse()) {
        return if_true->Eval(scope);
    } else {
        if (args.size() == 2) {
            return nullptr;
        }
        auto if_false = args[2];
        return if_false->Eval(scope);
    }
}

std::shared_ptr<Object> CheckNull::Apply(const std::shared_ptr<Scope>&,
                                         const std::vector<std::shared_ptr<Object>>& args) {
    if (args.size() != 1) {
        throw RuntimeError("Syntax error!");
    }
    if (args[0] != nullptr) {
        return std::make_shared<Boolean>(false);
    }
    return std::make_shared<Boolean>(true);
}

std::shared_ptr<Object> CheckPair::Apply(const std::shared_ptr<Scope>&,
                                         const std::vector<std::shared_ptr<Object>>& args) {
    if (args.size() != 1) {
        throw RuntimeError("Syntax error!");
    }
    return std::make_shared<Boolean>(IsCell(args[0]));
}

std::shared_ptr<Object> CheckNumber::Apply(const std::shared_ptr<Scope>&,
                                           const std::vector<std::shared_ptr<Object>>& args) {
    if (args.size() != 1) {
        throw RuntimeError("Syntax error!");
    }
    return std::make_shared<Boolean>(IsNumber(args[0]));
}

std::shared_ptr<Object> CheckBoolean::Apply(const std::shared_ptr<Scope>&,
                                            const std::vector<std::shared_ptr<Object>>& args) {
    if (args.size() != 1) {
        throw RuntimeError("Syntax error!");
    }
    return std::make_shared<Boolean>(IsSymbol(args[0]) && (AsSymbol(args[0])->GetName() == "#f" ||
                                                           AsSymbol(args[0])->GetName() == "#t"));
}

std::shared_ptr<Object> CheckSymbol::Apply(const std::shared_ptr<Scope>&,
                                           const std::vector<std::shared_ptr<Object>>& args) {
    if (args.size() != 1) {
        throw RuntimeError("Syntax error!");
    }
    return std::make_shared<Boolean>(IsSymbol(args[0]));
}

std::shared_ptr<Object> CheckList::Apply(const std::shared_ptr<Scope>&,
                                         const std::vector<std::shared_ptr<Object>>& args) {
    if (args.size() != 1) {
        throw RuntimeError("Syntax error!");
    }
    auto checker = args[0];
    while (checker) {
        if (!IsCell(checker)) {
            return std::make_shared<Boolean>(false);
        }
        checker = AsCell(checker)->GetSecond();
    }
    return std::make_shared<Boolean>(true);
}

std::shared_ptr<Object> Eq::Apply(const std::shared_ptr<Scope>&,
                                  const std::vector<std::shared_ptr<Object>>& args) {
    if (args.size() == 0) {
        throw RuntimeError("Syntax error!");
    }
    if (args[0] != nullptr) {
        return std::make_shared<Boolean>(false);
    }
    return std::make_shared<Boolean>(true);
}

std::shared_ptr<Object> Equal::Apply(const std::shared_ptr<Scope>&,
                                     const std::vector<std::shared_ptr<Object>>& args) {
    if (args.size() != 1) {
        throw RuntimeError("Syntax error!");
    }
    if (args[0] != nullptr) {
        return std::make_shared<Boolean>(false);
    }
    return std::make_shared<Boolean>(true);
}

std::shared_ptr<Object> IntegerEqual::Apply(const std::shared_ptr<Scope>&,
                                            const std::vector<std::shared_ptr<Object>>& args) {
    if (args.size() == 0) {
        return std::make_shared<Boolean>(true);
    }
    if (!IsNumber(args[0])) {
        throw RuntimeError("Syntax error!");
    }
    int64_t value = AsNumber(args[0])->GetValue();
    for (const auto& obj : args) {
        if (!IsNumber(obj)) {
            throw RuntimeError("Syntax error!");
        }
        if (value != AsNumber(obj)->GetValue()) {
            return std::make_shared<Boolean>(false);
        }
    }
    return std::make_shared<Boolean>(true);
}

std::shared_ptr<Object> Not::Apply(const std::shared_ptr<Scope>&,
                                   const std::vector<std::shared_ptr<Object>>& args) {
    if (args.size() != 1) {
        throw RuntimeError("Syntax error!");
    }
    if (args[0] == nullptr) {
        return std::make_shared<Boolean>(false);
    }
    if (args[0]->IsFalse()) {
        return std::make_shared<Boolean>(true);
    }
    return std::make_shared<Boolean>(false);
}

std::shared_ptr<Object> Equality::Apply(const std::shared_ptr<Scope>&,
                                        const std::vector<std::shared_ptr<Object>>& args) {
    if (args.size() == 0) {
        return std::make_shared<Boolean>(true);
    }
    if (!IsNumber(args[0])) {
        throw RuntimeError("Syntax error!");
    }
    int64_t value = AsNumber(args[0])->GetValue();
    for (const auto& obj : args) {
        if (!IsNumber(obj)) {
            throw RuntimeError("Syntax error!");
        }
        if (value != AsNumber(obj)->GetValue()) {
            return std::make_shared<Boolean>(false);
        }
    }
    return std::make_shared<Boolean>(true);
}

std::shared_ptr<Object> More::Apply(const std::shared_ptr<Scope>&,
                                    const std::vector<std::shared_ptr<Object>>& args) {
    for (size_t i = 1; i < args.size(); ++i) {
        if (!IsNumber(args[i]) || !IsNumber(args[i - 1])) {
            throw RuntimeError("Syntax error!");
        }
        if (AsNumber(args[i - 1])->GetValue() <= AsNumber(args[i])->GetValue()) {
            return std::make_shared<Boolean>(false);
        }
    }
    return std::make_shared<Boolean>(true);
}

std::shared_ptr<Object> Less::Apply(const std::shared_ptr<Scope>&,
                                    const std::vector<std::shared_ptr<Object>>& args) {
    for (size_t i = 1; i < args.size(); ++i) {
        if (!IsNumber(args[i]) || !IsNumber(args[i - 1])) {
            throw RuntimeError("Syntax error!");
        }
        if (AsNumber(args[i - 1])->GetValue() >= AsNumber(args[i])->GetValue()) {
            return std::make_shared<Boolean>(false);
        }
    }
    return std::make_shared<Boolean>(true);
}

std::shared_ptr<Object> MoreOrEqual::Apply(const std::shared_ptr<Scope>&,
                                           const std::vector<std::shared_ptr<Object>>& args) {
    for (size_t i = 1; i < args.size(); ++i) {
        if (!IsNumber(args[i]) || !IsNumber(args[i - 1])) {
            throw RuntimeError("Syntax error!");
        }
        if (AsNumber(args[i - 1])->GetValue() < AsNumber(args[i])->GetValue()) {
            return std::make_shared<Boolean>(false);
        }
    }
    return std::make_shared<Boolean>(true);
}

std::shared_ptr<Object> LessOrEqual::Apply(const std::shared_ptr<Scope>&,
                                           const std::vector<std::shared_ptr<Object>>& args) {
    for (size_t i = 1; i < args.size(); ++i) {
        if (!IsNumber(args[i]) || !IsNumber(args[i - 1])) {
            throw RuntimeError("Syntax error!");
        }
        if (AsNumber(args[i - 1])->GetValue() > AsNumber(args[i])->GetValue()) {
            return std::make_shared<Boolean>(false);
        }
    }
    return std::make_shared<Boolean>(true);
}

std::shared_ptr<Object> Min::Apply(const std::shared_ptr<Scope>&,
                                   const std::vector<std::shared_ptr<Object>>& args) {
    if (args.size() == 0) {
        throw RuntimeError("Syntax error!");
    }
    if (!IsNumber(args[0])) {
        throw RuntimeError("Syntax error!");
    }
    int64_t value = AsNumber(args[0])->GetValue();
    for (const auto& obj : args) {
        if (!IsNumber(obj)) {
            throw RuntimeError("Syntax error!");
        }
        int64_t current = AsNumber(obj)->GetValue();
        if (value > current) {
            value = current;
        }
    }
    return std::make_shared<Number>(value);
}

std::shared_ptr<Object> Max::Apply(const std::shared_ptr<Scope>&,
                                   const std::vector<std::shared_ptr<Object>>& args) {
    if (args.size() == 0) {
        throw RuntimeError("Syntax error!");
    }
    if (!IsNumber(args[0])) {
        throw RuntimeError("Syntax error!");
    }
    int64_t value = AsNumber(args[0])->GetValue();
    for (const auto& obj : args) {
        if (!IsNumber(obj)) {
            throw RuntimeError("Syntax error!");
        }
        int64_t current = AsNumber(obj)->GetValue();
        if (value < current) {
            value = current;
        }
    }
    return std::make_shared<Number>(value);
}

std::shared_ptr<Object> Abs::Apply(const std::shared_ptr<Scope>&,
                                   const std::vector<std::shared_ptr<Object>>& args) {
    if (args.size() != 1) {
        throw RuntimeError("Syntax error!");
    }
    if (!IsNumber(args[0])) {
        throw RuntimeError("Syntax error!");
    }
    return std::make_shared<Number>(std::abs(AsNumber(args[0])->GetValue()));
}

std::shared_ptr<Object> Cons::Apply(const std::shared_ptr<Scope>&,
                                    const std::vector<std::shared_ptr<Object>>& args) {
    if (args.size() != 2) {
        throw RuntimeError("Syntax error!");
    }
    auto new_cell = std::make_shared<Cell>();
    new_cell->SetFirst(args[0]);
    new_cell->SetSecond(args[1]);
    return new_cell;
}

std::shared_ptr<Object> Car::Apply(const std::shared_ptr<Scope>&,
                                   const std::vector<std::shared_ptr<Object>>& args) {
    if (args.size() != 1) {
        throw RuntimeError("Syntax error!");
    }
    if (!IsCell(args[0])) {
        throw RuntimeError("Syntax error!");
    }
    return AsCell(args[0])->GetFirst();
}

std::shared_ptr<Object> Cdr::Apply(const std::shared_ptr<Scope>&,
                                   const std::vector<std::shared_ptr<Object>>& args) {
    if (args.size() != 1) {
        throw RuntimeError("Syntax error!");
    }
    if (!IsCell(args[0])) {
        throw RuntimeError("Syntax error!");
    }
    return AsCell(args[0])->GetSecond();
}

std::shared_ptr<Object> SetCar::Apply(const std::shared_ptr<Scope>&,
                                      const std::vector<std::shared_ptr<Object>>& args) {
    if (args.size() != 2) {
        throw RuntimeError("Syntax error!");
    }
    if (!IsCell(args[0])) {
        throw RuntimeError("Syntax error!");
    }
    AsCell(args[0])->SetFirst(args[1]);
    return args[0];
}

std::shared_ptr<Object> SetCdr::Apply(const std::shared_ptr<Scope>&,
                                      const std::vector<std::shared_ptr<Object>>& args) {
    if (args.size() != 2) {
        throw RuntimeError("Syntax error!");
    }
    if (!IsCell(args[0])) {
        throw RuntimeError("Syntax error!");
    }
    AsCell(args[0])->SetSecond(args[1]);
    return args[0];
}

std::shared_ptr<Object> List::Apply(const std::shared_ptr<Scope>&,
                                    const std::vector<std::shared_ptr<Object>>& args) {
    if (args.size() == 0) {
        return nullptr;
    }
    auto new_cell = std::make_shared<Cell>();
    auto res = new_cell;
    for (size_t ind = 0; ind < args.size(); ++ind) {
        new_cell->SetFirst(args[ind]);
        if (ind != args.size() - 1) {
            new_cell->SetSecond(std::make_shared<Cell>());
        }
        new_cell = AsCell(new_cell->GetSecond());
    }
    return res;
}

std::shared_ptr<Object> ListRef::Apply(const std::shared_ptr<Scope>&,
                                       const std::vector<std::shared_ptr<Object>>& args) {
    if (args.size() != 2) {
        throw RuntimeError("Wrong number of arguments");
    }
    if (!IsCell(args[0]) && !IsNumber(args[1])) {
        throw RuntimeError("Arguments must be list and a number");
    }
    auto scope = args[0];
    auto value = AsNumber(args[1])->GetValue();
    while (value != 0) {
        if (!IsCell(scope)) {
            throw RuntimeError("List is too short");
        }
        scope = AsCell(scope)->GetSecond();
        --value;
    }
    if (!IsCell(scope)) {
        throw RuntimeError("List is too short");
    }
    return AsCell(scope)->GetFirst();
}

std::shared_ptr<Object> ListTail::Apply(const std::shared_ptr<Scope>&,
                                        const std::vector<std::shared_ptr<Object>>& args) {
    if (args.size() != 2) {
        throw RuntimeError("Syntax error!");
    }
    if (!IsCell(args[0]) && !IsNumber(args[1])) {
        throw RuntimeError("Syntax error!");
    }
    auto scope = args[0];
    auto value = AsNumber(args[1])->GetValue();
    while (value != 0) {
        if (!IsCell(scope)) {
            throw RuntimeError("Syntax error!");
        }
        scope = AsCell(scope)->GetSecond();
        --value;
    }
    if (!IsCell(scope) && scope) {
        throw RuntimeError("Syntax error!");
    } else if (!scope) {
        return nullptr;
    }
    return AsCell(scope);
}

std::shared_ptr<Object> And::Apply(const std::shared_ptr<Scope>& scope,
                                   const std::vector<std::shared_ptr<Object>>& args) {
    for (size_t ind = 0; ind < args.size(); ++ind) {
        auto res = args[ind]->Eval(scope);
        if (res->IsFalse()) {
            return std::make_shared<Boolean>(false);
        }
        if (ind == args.size() - 1) {
            return res;
        }
    }
    return std::make_shared<Boolean>(true);
}

std::shared_ptr<Object> Or::Apply(const std::shared_ptr<Scope>& scope,
                                  const std::vector<std::shared_ptr<Object>>& args) {
    for (size_t ind = 0; ind < args.size(); ++ind) {
        auto res = args[ind]->Eval(scope);
        if (!res->IsFalse()) {
            return res;
        }
    }
    return std::make_shared<Boolean>(false);
}

std::shared_ptr<Object> Define::Apply(const std::shared_ptr<Scope>& scope,
                                      const std::vector<std::shared_ptr<Object>>& args) {
    if (args.size() != 2) {
        throw SyntaxError("Wrong number of arguments!");
    }
    if (IsSymbol(args[0])) {
        scope->variables_[AsSymbol(args[0])->GetName()] = args[1]->Eval(scope);
    } else if (IsCell(args[0])) {
        auto new_lambda_function = std::make_shared<LambdaFunction>();
        new_lambda_function->SetArgs(ToVector(AsCell(args[0])->GetSecond()));
        for (auto& arg : new_lambda_function->GetArgs()) {
            if (!IsSymbol(arg)) {
                throw SyntaxError("wrong argument name");
            }
        }
        new_lambda_function->SetScope(scope);
        for (size_t ind = 1; ind < args.size(); ++ind) {
            new_lambda_function->AddToBody(args[ind]);
        }
        scope->variables_[AsSymbol(AsCell(args[0])->GetFirst())->GetName()] = new_lambda_function;
    }
    return nullptr;
}

std::shared_ptr<Object> Set::Apply(const std::shared_ptr<Scope>& scope,
                                   const std::vector<std::shared_ptr<Object>>& args) {
    if (args.size() != 2) {
        throw SyntaxError("Syntax error!");
    }
    if (IsSymbol(args[0])) {
        scope->Lookup(AsSymbol(args[0])->GetName());
        scope->variables_[AsSymbol(args[0])->GetName()] = args[1]->Eval(scope);
    } else {
        throw RuntimeError("something bad happened");
    }
    return nullptr;
}

std::shared_ptr<Object> Lambda::Apply(const std::shared_ptr<Scope>& scope,
                                      const std::vector<std::shared_ptr<Object>>& args) {
    if (args.size() < 2) {
        throw SyntaxError("Syntax error!");
    }
    if (!IsCell(args[0]) && args[0]) {
        throw SyntaxError("Bad argument list!");
    }
    auto new_lambda_function = std::make_shared<LambdaFunction>();
    new_lambda_function->SetArgs(ToVector(args[0]));
    for (auto& arg : new_lambda_function->GetArgs()) {
        if (!IsSymbol(arg)) {
            throw SyntaxError("wrong argument name");
        }
    }
    auto local_scope = std::make_shared<Scope>();
    local_scope->parent_ = scope;
    new_lambda_function->SetScope(local_scope);
    for (size_t ind = 1; ind < args.size(); ++ind) {
        new_lambda_function->AddToBody(args[ind]);
    }
    return new_lambda_function;
}

std::shared_ptr<Object> LambdaFunction::Apply(const std::shared_ptr<Scope>& scope,
                                              const std::vector<std::shared_ptr<Object>>& args) {
    if (args_.size() != args.size()) {
        throw RuntimeError("wrong number of arguments");
    }
    for (size_t ind = 0; ind < args.size(); ++ind) {
        current_scope_->variables_[AsSymbol(args_[ind])->GetName()] = args[ind]->Eval(scope);
    }
    for (size_t ind = 0; ind < body_.size(); ++ind) {
        if (ind != body_.size() - 1) {
            body_[ind]->Eval(current_scope_);
        } else {
            return body_[ind]->Eval(current_scope_);
        }
    }
    return nullptr;
}

SyntaxError::SyntaxError(const std::string& what) : std::runtime_error(what) {
}

NameError::NameError(const std::string& name) : std::runtime_error("variable not found: " + name) {
}

RuntimeError::RuntimeError(const std::string& what) : std::runtime_error(what) {
}

bool IsNumber(const std::shared_ptr<Object>& obj) {
    if (obj) {
        return Types::numberType == obj.get()->ID();
    }
    return false;
}

std::shared_ptr<Number> AsNumber(const std::shared_ptr<Object>& obj) {
    if (IsNumber(obj)) {
        return std::static_pointer_cast<Number>(obj);
    }
    return nullptr;
}

bool IsCell(const std::shared_ptr<Object>& obj) {
    if (obj) {
        return Types::cellType == obj->ID();
    }
    return false;
}

std::shared_ptr<Cell> AsCell(const std::shared_ptr<Object>& obj) {
    if (IsCell(obj)) {
        return std::static_pointer_cast<Cell>(obj);
    }
    return nullptr;
}

bool IsSymbol(const std::shared_ptr<Object>& obj) {
    if (obj) {
        return Types::symbolType == obj->ID();
    }
    return false;
}

std::shared_ptr<Symbol> AsSymbol(const std::shared_ptr<Object>& obj) {
    if (IsSymbol(obj)) {
        return std::static_pointer_cast<Symbol>(obj);
    }
    return nullptr;
}

std::shared_ptr<Object> ReadProper(Tokenizer* tokenizer) {
    if (tokenizer->IsEnd()) {
        return nullptr;
    }
    auto current_object = tokenizer->GetToken();
    if (SymbolToken* symbol = std::get_if<SymbolToken>(&current_object)) {
        tokenizer->Next();
        if (symbol->name == "#t") {
            return std::make_shared<Boolean>(true);
        } else if (symbol->name == "#f") {
            return std::make_shared<Boolean>(false);
        }
        return std::make_shared<Symbol>(symbol->name);
    } else if (ConstantToken* constant = std::get_if<ConstantToken>(&current_object)) {
        tokenizer->Next();
        return std::make_shared<Number>(constant->value);
    } else if (std::holds_alternative<QuoteToken>(current_object)) {
        tokenizer->Next();
        auto new_cell = std::make_shared<Cell>();
        auto list = std::make_shared<Cell>();
        new_cell->SetFirst(std::make_shared<Symbol>("quote"));
        list->SetFirst(ReadProper(tokenizer));
        new_cell->SetSecond(list);
        return new_cell;
    } else if (std::holds_alternative<DotToken>(current_object)) {
        throw SyntaxError("Unexpected symbol");
    } else if (BracketToken* bracket = std::get_if<BracketToken>(&current_object)) {
        if (*bracket == BracketToken::CLOSE) {
            throw SyntaxError("Unexpected closing parentheses");
        }
        tokenizer->Next();
        return ReadList(tokenizer);
    }
    throw SyntaxError("Unexpected symbol");
}

std::shared_ptr<Object> ReadList(Tokenizer* tokenizer) {
    if (tokenizer->IsEnd()) {
        throw SyntaxError("Input not complete");
    }
    std::shared_ptr<Object> head = nullptr;
    std::shared_ptr<Cell> tail = nullptr;
    while (!tokenizer->IsEnd()) {
        auto current_token = tokenizer->GetToken();
        if (std::holds_alternative<BracketToken>(current_token) &&
            std::get<BracketToken>(current_token) == BracketToken::CLOSE) {
            tokenizer->Next();
            return head;
        } else if (std::holds_alternative<DotToken>(current_token)) {
            tokenizer->Next();
            if (tail == nullptr) {
                throw SyntaxError("Improper list syntax");
            }
            tail->SetSecond(ReadProper(tokenizer));
            if (!std::holds_alternative<BracketToken>(tokenizer->GetToken()) ||
                std::get<BracketToken>(tokenizer->GetToken()) != BracketToken::CLOSE) {
                throw SyntaxError("Improper list syntax");
            }
        } else {
            auto current_object = ReadProper(tokenizer);
            auto new_cell = std::make_shared<Cell>();
            new_cell->SetFirst(current_object);
            if (head == nullptr) {
                head = new_cell;
                tail = new_cell;
            } else {
                tail->SetSecond(new_cell);
                tail = new_cell;
            }
        }
    }
    throw SyntaxError("Unmatched opening parentheses");
}

std::shared_ptr<Object> Read(Tokenizer* tokenizer) {
    auto obj = ReadProper(tokenizer);
    if (!tokenizer->IsEnd()) {
        throw SyntaxError("ImproperSyntax");
    }
    return obj;
}
