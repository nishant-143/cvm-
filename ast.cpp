#include "ast.h"
#include <iostream>

static const char* nodeTypeName(NodeType t) {
    switch (t) {
        case NodeType::Program:       return "Program";
        case NodeType::NumberLiteral: return "NumberLiteral";
        case NodeType::BoolLiteral:   return "BoolLiteral";
        case NodeType::Identifier:    return "Identifier";
        case NodeType::BinaryOp:      return "BinaryOp";
        case NodeType::Assignment:    return "Assignment";
        case NodeType::VarDecl:       return "VarDecl";
        case NodeType::IfStmt:        return "IfStmt";
        case NodeType::WhileStmt:     return "WhileStmt";
        case NodeType::PrintStmt:     return "PrintStmt";
        case NodeType::InputStmt:     return "InputStmt";
        case NodeType::Block:         return "Block";
        default:                      return "Unknown";
    }
}

void printAST(const ASTNode* node, int indent) {
    if (!node) return;
    std::string pad(indent * 2, ' ');
    std::cout << pad << "[" << nodeTypeName(node->type) << "]";

    if (node->type == NodeType::NumberLiteral)
        std::cout << " " << node->numValue;
    else if (node->type == NodeType::BoolLiteral)
        std::cout << " " << (node->boolValue ? "true" : "false");
    else if (!node->strValue.empty())
        std::cout << " \"" << node->strValue << "\"";

    std::cout << "\n";
    for (const auto& child : node->children)
        printAST(child.get(), indent + 1);
}