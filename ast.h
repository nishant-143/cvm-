#pragma once
#include <memory>
#include <string>
#include <vector>

// Forward declarations
struct ASTNode;
using ASTNodePtr = std::unique_ptr<ASTNode>;

enum class NodeType {
    Program,
    NumberLiteral,
    BoolLiteral,
    Identifier,
    BinaryOp,
    Assignment,
    VarDecl,
    IfStmt,
    WhileStmt,
    PrintStmt,
    InputStmt,
    Block,
};

struct ASTNode {
    NodeType type;
    // For literals
    double numValue = 0;
    bool   boolValue = false;
    std::string strValue;   // identifier name or operator
    // Children
    std::vector<ASTNodePtr> children;

    ASTNode(NodeType t) : type(t) {}
};

// Pretty-print the AST
void printAST(const ASTNode* node, int indent = 0);
