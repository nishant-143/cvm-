#pragma once
#include "lexer.h"
#include "ast.h"
#include <vector>

class Parser {
public:
    explicit Parser(const std::vector<Token>& tokens);
    ASTNodePtr parse();   // returns Program node

private:
    std::vector<Token> tokens;
    size_t pos;

    Token& peek(int offset = 0);
    Token  consume();
    Token  expect(TokenType t, const std::string& msg);
    bool   check(TokenType t);
    bool   match(TokenType t);

    ASTNodePtr parseStatement();
    ASTNodePtr parseVarDecl();
    ASTNodePtr parseIfStmt();
    ASTNodePtr parseWhileStmt();
    ASTNodePtr parsePrintStmt();
    ASTNodePtr parseInputStmt();
    ASTNodePtr parseBlock();
    ASTNodePtr parseExprStatement();

    ASTNodePtr parseExpression();
    ASTNodePtr parseAssignment();
    ASTNodePtr parseComparison();
    ASTNodePtr parseTerm();
    ASTNodePtr parseFactor();
    ASTNodePtr parseUnary();
    ASTNodePtr parsePrimary();
};
