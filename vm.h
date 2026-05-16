#pragma once
#include "compiler.h"
#include <vector>
#include <variant>
#include <string>

// Runtime value: either a double or bool
using Value = std::variant<double, bool>;

std::string valueToString(const Value& v);

class VM {
public:
    void execute(const Chunk& chunk);

private:
    std::vector<Value> stack;
    std::vector<Value> slots;   // variable storage

    void push(Value v);
    Value pop();
    Value top();
};
