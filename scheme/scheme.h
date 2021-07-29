#pragma once

#include <memory>
#include <string>
#include "parser.h"
#include <functional>
#include <sstream>

class SchemeInterpretor {
public:
    SchemeInterpretor();

    ~SchemeInterpretor();

    std::shared_ptr<Object> Eval(std::shared_ptr<Object> in);

private:
    std::shared_ptr<Scope> global_scope_;
};
