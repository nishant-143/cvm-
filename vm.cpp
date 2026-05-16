#include "vm.h"
#include <iostream>
#include <stdexcept>
#include <cstring>

std::string valueToString(const Value& v) {
    if (std::holds_alternative<double>(v)) {
        double d = std::get<double>(v);
        if (d == static_cast<long long>(d))
            return std::to_string(static_cast<long long>(d));
        return std::to_string(d);
    }
    return std::get<bool>(v) ? "true" : "false";
}

void VM::push(Value v) { stack.push_back(v); }

Value VM::pop() {
    if (stack.empty()) throw std::runtime_error("VM stack underflow");
    Value v = stack.back(); stack.pop_back(); return v;
}

Value VM::top() {
    if (stack.empty()) throw std::runtime_error("VM stack empty");
    return stack.back();
}

void VM::execute(const Chunk& chunk) {
    slots.assign(chunk.numSlots, Value{0.0});
    const auto& code = chunk.code;
    size_t ip = 0; // instruction pointer

    auto readDouble = [&]() -> double {
        double d; std::memcpy(&d, &code[ip], 8); ip += 8; return d;
    };
    auto readUint16 = [&]() -> uint16_t {
        uint16_t v = (static_cast<uint16_t>(code[ip]) << 8) | code[ip+1];
        ip += 2; return v;
    };

    while (ip < code.size()) {
        OpCode op = static_cast<OpCode>(code[ip++]);

        switch (op) {
            case OpCode::PUSH_NUM:
                push(readDouble());
                break;

            case OpCode::PUSH_BOOL:
                push(static_cast<bool>(code[ip++]));
                break;

            case OpCode::POP:
                pop();
                break;

            case OpCode::LOAD: {
                int slot = code[ip++];
                push(slots[slot]);
                break;
            }

            case OpCode::STORE: {
                int slot = code[ip++];
                slots[slot] = top(); // don't pop; STORE leaves value on stack
                pop();
                break;
            }

            case OpCode::ADD: {
                Value b = pop(), a = pop();
                push(std::get<double>(a) + std::get<double>(b));
                break;
            }
            case OpCode::SUB: {
                Value b = pop(), a = pop();
                push(std::get<double>(a) - std::get<double>(b));
                break;
            }
            case OpCode::MUL: {
                Value b = pop(), a = pop();
                push(std::get<double>(a) * std::get<double>(b));
                break;
            }
            case OpCode::DIV: {
                Value b = pop(), a = pop();
                double db = std::get<double>(b);
                if (db == 0) throw std::runtime_error("Division by zero");
                push(std::get<double>(a) / db);
                break;
            }

            case OpCode::EQ: {
                Value b = pop(), a = pop();
                push(a == b);
                break;
            }
            case OpCode::NEQ: {
                Value b = pop(), a = pop();
                push(a != b);
                break;
            }
            case OpCode::LT: {
                Value b = pop(), a = pop();
                push(std::get<double>(a) < std::get<double>(b));
                break;
            }
            case OpCode::GT: {
                Value b = pop(), a = pop();
                push(std::get<double>(a) > std::get<double>(b));
                break;
            }

            case OpCode::JUMP:
                ip = readUint16();
                break;

            case OpCode::JUMP_IF_FALSE: {
                uint16_t target = readUint16();
                Value cond = pop();
                bool isFalse = std::holds_alternative<bool>(cond)
                    ? !std::get<bool>(cond)
                    : std::get<double>(cond) == 0.0;
                if (isFalse) ip = target;
                break;
            }

            case OpCode::PRINT:
                std::cout << valueToString(pop()) << "\n";
                break;

            case OpCode::INPUT: {
                int slot = code[ip++];
                double val;
                std::cout << ">> ";
                std::cin >> val;
                slots[slot] = val;
                break;
            }

            case OpCode::HALT:
                return;

            default:
                throw std::runtime_error("Unknown opcode: " + std::to_string(static_cast<int>(op)));
        }
    }
}
