#pragma once
#include "ast.h"
#include <vector>
#include <cstdint>
#include <string>
#include <unordered_map>

// ── Opcodes ──────────────────────────────────────────────────────────────────
enum class OpCode : uint8_t {
    PUSH_NUM,     // push a double constant   (8 bytes follow)
    PUSH_BOOL,    // push a bool constant     (1 byte follows: 0/1)
    POP,          // discard top of stack

    LOAD,         // load variable by index   (1 byte: slot)
    STORE,        // store into slot          (1 byte: slot)

    ADD, SUB, MUL, DIV,
    EQ, NEQ, LT, GT,

    JUMP,         // unconditional jump       (2 bytes: offset uint16)
    JUMP_IF_FALSE,// pop & jump if false      (2 bytes: offset uint16)

    PRINT,        // pop & print top
    INPUT,        // push user input as num   (1 byte: slot to store)

    HALT,
};

std::string opcodeName(OpCode op);

// ── Bytecode chunk ───────────────────────────────────────────────────────────
struct Chunk {
    std::vector<uint8_t> code;   // raw bytecode bytes
    int numSlots = 0;            // number of variable slots needed
    std::vector<std::string> slotNames; // slot index → variable name (debug)

    void emit(uint8_t byte);
    void emitDouble(double d);
    size_t emitJump(OpCode op);          // emits JUMP/JUMP_IF_FALSE + placeholder
    void patchJump(size_t jumpPos);      // patches placeholder to current position
    void disassemble() const;
};

// ── Compiler ─────────────────────────────────────────────────────────────────
class Compiler {
public:
    Chunk compile(const ASTNode* program);

private:
    Chunk chunk;
    std::unordered_map<std::string, int> slots;

    int allocSlot(const std::string& name);
    int getSlot(const std::string& name);

    void compileNode(const ASTNode* node);
    void compileExpr(const ASTNode* node);
};
