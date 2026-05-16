#include "compiler.h"
#include <stdexcept>
#include <cstring>
#include <iostream>
#include <iomanip>

// ── Opcode name helper ────────────────────────────────────────────────────────
std::string opcodeName(OpCode op) {
    switch (op) {
        case OpCode::PUSH_NUM:      return "PUSH_NUM";
        case OpCode::PUSH_BOOL:     return "PUSH_BOOL";
        case OpCode::POP:           return "POP";
        case OpCode::LOAD:          return "LOAD";
        case OpCode::STORE:         return "STORE";
        case OpCode::ADD:           return "ADD";
        case OpCode::SUB:           return "SUB";
        case OpCode::MUL:           return "MUL";
        case OpCode::DIV:           return "DIV";
        case OpCode::EQ:            return "EQ";
        case OpCode::NEQ:           return "NEQ";
        case OpCode::LT:            return "LT";
        case OpCode::GT:            return "GT";
        case OpCode::JUMP:          return "JUMP";
        case OpCode::JUMP_IF_FALSE: return "JUMP_IF_FALSE";
        case OpCode::PRINT:         return "PRINT";
        case OpCode::INPUT:         return "INPUT";
        case OpCode::HALT:          return "HALT";
        default:                    return "???";
    }
}

// ── Chunk helpers ─────────────────────────────────────────────────────────────
void Chunk::emit(uint8_t byte) { code.push_back(byte); }

void Chunk::emitDouble(double d) {
    uint8_t buf[8];
    std::memcpy(buf, &d, 8);
    for (int i = 0; i < 8; i++) code.push_back(buf[i]);
}

size_t Chunk::emitJump(OpCode op) {
    emit(static_cast<uint8_t>(op));
    size_t jumpPos = code.size();
    emit(0xFF); emit(0xFF); // placeholder 2-byte offset
    return jumpPos;
}

void Chunk::patchJump(size_t jumpPos) {
    // target = current end of code
    uint16_t target = static_cast<uint16_t>(code.size());
    code[jumpPos]     = static_cast<uint8_t>(target >> 8);
    code[jumpPos + 1] = static_cast<uint8_t>(target & 0xFF);
}

void Chunk::disassemble() const {
    std::cout << "\n=== Bytecode Disassembly ===\n";
    size_t i = 0;
    while (i < code.size()) {
        std::cout << std::setw(4) << i << "  ";
        OpCode op = static_cast<OpCode>(code[i++]);
        std::cout << opcodeName(op);
        switch (op) {
            case OpCode::PUSH_NUM: {
                double d; std::memcpy(&d, &code[i], 8); i += 8;
                std::cout << "  " << d;
                break;
            }
            case OpCode::PUSH_BOOL:
                std::cout << "  " << (code[i++] ? "true" : "false");
                break;
            case OpCode::LOAD:
            case OpCode::STORE: {
                int slot = code[i++];
                std::cout << "  slot[" << slot << "]";
                if (slot < (int)slotNames.size())
                    std::cout << " (" << slotNames[slot] << ")";
                break;
            }
            case OpCode::JUMP:
            case OpCode::JUMP_IF_FALSE: {
                uint16_t target = (static_cast<uint16_t>(code[i]) << 8) | code[i+1];
                i += 2;
                std::cout << "  -> " << target;
                break;
            }
            case OpCode::INPUT: {
                int slot = code[i++];
                std::cout << "  slot[" << slot << "]";
                if (slot < (int)slotNames.size())
                    std::cout << " (" << slotNames[slot] << ")";
                break;
            }
            default: break;
        }
        std::cout << "\n";
    }
    std::cout << "===========================\n\n";
}

// ── Compiler ─────────────────────────────────────────────────────────────────
int Compiler::allocSlot(const std::string& name) {
    if (slots.count(name)) return slots[name];
    int idx = chunk.numSlots++;
    slots[name] = idx;
    chunk.slotNames.push_back(name);
    return idx;
}

int Compiler::getSlot(const std::string& name) {
    auto it = slots.find(name);
    if (it == slots.end())
        throw std::runtime_error("Undefined variable '" + name + "'");
    return it->second;
}

Chunk Compiler::compile(const ASTNode* program) {
    compileNode(program);
    chunk.emit(static_cast<uint8_t>(OpCode::HALT));
    return std::move(chunk);
}

void Compiler::compileNode(const ASTNode* node) {
    switch (node->type) {

        case NodeType::Program:
        case NodeType::Block:
            for (const auto& child : node->children)
                compileNode(child.get());
            break;

        case NodeType::VarDecl: {
            int slot = allocSlot(node->strValue);
            compileExpr(node->children[0].get());
            chunk.emit(static_cast<uint8_t>(OpCode::STORE));
            chunk.emit(static_cast<uint8_t>(slot));
            break;
        }

        case NodeType::Assignment: {
            int slot = getSlot(node->strValue);
            compileExpr(node->children[0].get());
            chunk.emit(static_cast<uint8_t>(OpCode::STORE));
            chunk.emit(static_cast<uint8_t>(slot));
            break;
        }

        case NodeType::PrintStmt:
            compileExpr(node->children[0].get());
            chunk.emit(static_cast<uint8_t>(OpCode::PRINT));
            break;

        case NodeType::InputStmt: {
            int slot = getSlot(node->strValue);  // variable must be declared first
            chunk.emit(static_cast<uint8_t>(OpCode::INPUT));
            chunk.emit(static_cast<uint8_t>(slot));
            break;
        }

        case NodeType::IfStmt: {
            // condition
            compileExpr(node->children[0].get());
            size_t jumpToElse = chunk.emitJump(OpCode::JUMP_IF_FALSE);
            // then branch
            compileNode(node->children[1].get());
            if (node->children.size() == 3) {
                // jump over else
                size_t jumpOverElse = chunk.emitJump(OpCode::JUMP);
                chunk.patchJump(jumpToElse);
                compileNode(node->children[2].get());
                chunk.patchJump(jumpOverElse);
            } else {
                chunk.patchJump(jumpToElse);
            }
            break;
        }

        case NodeType::WhileStmt: {
            size_t loopStart = chunk.code.size();
            compileExpr(node->children[0].get());
            size_t exitJump = chunk.emitJump(OpCode::JUMP_IF_FALSE);
            compileNode(node->children[1].get());
            // jump back to loop start
            chunk.emit(static_cast<uint8_t>(OpCode::JUMP));
            uint16_t target = static_cast<uint16_t>(loopStart);
            chunk.emit(static_cast<uint8_t>(target >> 8));
            chunk.emit(static_cast<uint8_t>(target & 0xFF));
            chunk.patchJump(exitJump);
            break;
        }

        // Expression as statement — compile and discard result
        default:
            compileExpr(node);
            chunk.emit(static_cast<uint8_t>(OpCode::POP));
            break;
    }
}

void Compiler::compileExpr(const ASTNode* node) {
    switch (node->type) {
        case NodeType::NumberLiteral:
            chunk.emit(static_cast<uint8_t>(OpCode::PUSH_NUM));
            chunk.emitDouble(node->numValue);
            break;

        case NodeType::BoolLiteral:
            chunk.emit(static_cast<uint8_t>(OpCode::PUSH_BOOL));
            chunk.emit(node->boolValue ? 1 : 0);
            break;

        case NodeType::Identifier: {
            int slot = getSlot(node->strValue);
            chunk.emit(static_cast<uint8_t>(OpCode::LOAD));
            chunk.emit(static_cast<uint8_t>(slot));
            break;
        }

        case NodeType::BinaryOp: {
            compileExpr(node->children[0].get());
            compileExpr(node->children[1].get());
            const std::string& op = node->strValue;
            if      (op == "+")  chunk.emit(static_cast<uint8_t>(OpCode::ADD));
            else if (op == "-")  chunk.emit(static_cast<uint8_t>(OpCode::SUB));
            else if (op == "*")  chunk.emit(static_cast<uint8_t>(OpCode::MUL));
            else if (op == "/")  chunk.emit(static_cast<uint8_t>(OpCode::DIV));
            else if (op == "==") chunk.emit(static_cast<uint8_t>(OpCode::EQ));
            else if (op == "!=") chunk.emit(static_cast<uint8_t>(OpCode::NEQ));
            else if (op == "<")  chunk.emit(static_cast<uint8_t>(OpCode::LT));
            else if (op == ">")  chunk.emit(static_cast<uint8_t>(OpCode::GT));
            else throw std::runtime_error("Unknown binary operator: " + op);
            break;
        }

        case NodeType::Assignment: {
            int slot = getSlot(node->strValue);
            compileExpr(node->children[0].get());
            chunk.emit(static_cast<uint8_t>(OpCode::STORE));
            chunk.emit(static_cast<uint8_t>(slot));
            // leave value on stack (assignment is also an expression)
            chunk.emit(static_cast<uint8_t>(OpCode::LOAD));
            chunk.emit(static_cast<uint8_t>(slot));
            break;
        }

        default:
            throw std::runtime_error("Cannot compile node as expression");
    }
}
