#include "parser.h"
#include <stdexcept>

Parser::Parser(const std::vector<Token>& tokens) : tokens(tokens), pos(0) {}

Token& Parser::peek(int offset) {
    size_t idx = pos + offset;
    if (idx >= tokens.size()) return tokens.back();
    return tokens[idx];
}

Token Parser::consume() { return tokens[pos++]; }

Token Parser::expect(TokenType t, const std::string& msg) {
    if (!check(t)) throw std::runtime_error(msg + " (got '" + peek().value + "' at token " + std::to_string(pos) + ")");
    return consume();
}

bool Parser::check(TokenType t) { return peek().type == t; }

bool Parser::match(TokenType t) {
    if (check(t)) { consume(); return true; }
    return false;
}

// ── Top-level ────────────────────────────────────────────────────────────────
ASTNodePtr Parser::parse() {
    auto prog = std::make_unique<ASTNode>(NodeType::Program);
    while (!check(TokenType::EOF_TOKEN))
        prog->children.push_back(parseStatement());
    return prog;
}

// ── Statements ───────────────────────────────────────────────────────────────
ASTNodePtr Parser::parseStatement() {
    if (check(TokenType::LET))   return parseVarDecl();
    if (check(TokenType::IF))    return parseIfStmt();
    if (check(TokenType::WHILE)) return parseWhileStmt();
    if (check(TokenType::PRINT)) return parsePrintStmt();
    if (check(TokenType::INPUT)) return parseInputStmt();
    if (check(TokenType::LBRACE)) return parseBlock();
    return parseExprStatement();
}

ASTNodePtr Parser::parseVarDecl() {
    expect(TokenType::LET, "Expected 'let'");
    Token name = expect(TokenType::IDENTIFIER, "Expected variable name after 'let'");
    expect(TokenType::EQUAL, "Expected '=' after variable name");
    auto expr = parseExpression();
    expect(TokenType::SEMICOLON, "Expected ';' after variable declaration");

    auto node = std::make_unique<ASTNode>(NodeType::VarDecl);
    node->strValue = name.value;
    node->children.push_back(std::move(expr));
    return node;
}

ASTNodePtr Parser::parseIfStmt() {
    expect(TokenType::IF, "Expected 'if'");
    expect(TokenType::LPAREN, "Expected '(' after 'if'");
    auto cond = parseExpression();
    expect(TokenType::RPAREN, "Expected ')' after if condition");
    auto thenBranch = parseStatement();

    auto node = std::make_unique<ASTNode>(NodeType::IfStmt);
    node->children.push_back(std::move(cond));
    node->children.push_back(std::move(thenBranch));

    if (match(TokenType::ELSE))
        node->children.push_back(parseStatement());

    return node;
}

ASTNodePtr Parser::parseWhileStmt() {
    expect(TokenType::WHILE, "Expected 'while'");
    expect(TokenType::LPAREN, "Expected '(' after 'while'");
    auto cond = parseExpression();
    expect(TokenType::RPAREN, "Expected ')' after while condition");
    auto body = parseStatement();

    auto node = std::make_unique<ASTNode>(NodeType::WhileStmt);
    node->children.push_back(std::move(cond));
    node->children.push_back(std::move(body));
    return node;
}

ASTNodePtr Parser::parsePrintStmt() {
    expect(TokenType::PRINT, "Expected 'print'");
    expect(TokenType::LPAREN, "Expected '(' after 'print'");
    auto expr = parseExpression();
    expect(TokenType::RPAREN, "Expected ')' after print expression");
    expect(TokenType::SEMICOLON, "Expected ';' after print statement");

    auto node = std::make_unique<ASTNode>(NodeType::PrintStmt);
    node->children.push_back(std::move(expr));
    return node;
}

ASTNodePtr Parser::parseInputStmt() {
    expect(TokenType::INPUT, "Expected 'input'");
    expect(TokenType::LPAREN, "Expected '(' after 'input'");
    Token name = expect(TokenType::IDENTIFIER, "Expected variable name in input()");
    expect(TokenType::RPAREN, "Expected ')' after input variable");
    expect(TokenType::SEMICOLON, "Expected ';' after input statement");

    auto node = std::make_unique<ASTNode>(NodeType::InputStmt);
    node->strValue = name.value;
    return node;
}

ASTNodePtr Parser::parseBlock() {
    expect(TokenType::LBRACE, "Expected '{'");
    auto block = std::make_unique<ASTNode>(NodeType::Block);
    while (!check(TokenType::RBRACE) && !check(TokenType::EOF_TOKEN))
        block->children.push_back(parseStatement());
    expect(TokenType::RBRACE, "Expected '}'");
    return block;
}

ASTNodePtr Parser::parseExprStatement() {
    auto expr = parseExpression();
    expect(TokenType::SEMICOLON, "Expected ';' after expression");
    return expr;
}

// ── Expressions (Pratt-style precedence) ─────────────────────────────────────
ASTNodePtr Parser::parseExpression() { return parseAssignment(); }

ASTNodePtr Parser::parseAssignment() {
    // Check for identifier = expr (without consuming yet)
    if (check(TokenType::IDENTIFIER) && pos + 1 < tokens.size() && tokens[pos+1].type == TokenType::EQUAL) {
        Token name = consume(); // identifier
        consume();              // '='
        auto val = parseExpression();

        auto node = std::make_unique<ASTNode>(NodeType::Assignment);
        node->strValue = name.value;
        node->children.push_back(std::move(val));
        return node;
    }
    return parseComparison();
}

ASTNodePtr Parser::parseComparison() {
    auto left = parseTerm();
    while (check(TokenType::EQ_EQ) || check(TokenType::BANG_EQ) ||
           check(TokenType::LESS)  || check(TokenType::GREATER)) {
        Token op = consume();
        auto right = parseTerm();
        auto node = std::make_unique<ASTNode>(NodeType::BinaryOp);
        node->strValue = op.value;
        node->children.push_back(std::move(left));
        node->children.push_back(std::move(right));
        left = std::move(node);
    }
    return left;
}

ASTNodePtr Parser::parseTerm() {
    auto left = parseFactor();
    while (check(TokenType::PLUS) || check(TokenType::MINUS)) {
        Token op = consume();
        auto right = parseFactor();
        auto node = std::make_unique<ASTNode>(NodeType::BinaryOp);
        node->strValue = op.value;
        node->children.push_back(std::move(left));
        node->children.push_back(std::move(right));
        left = std::move(node);
    }
    return left;
}

ASTNodePtr Parser::parseFactor() {
    auto left = parseUnary();
    while (check(TokenType::STAR) || check(TokenType::SLASH)) {
        Token op = consume();
        auto right = parseUnary();
        auto node = std::make_unique<ASTNode>(NodeType::BinaryOp);
        node->strValue = op.value;
        node->children.push_back(std::move(left));
        node->children.push_back(std::move(right));
        left = std::move(node);
    }
    return left;
}

ASTNodePtr Parser::parseUnary() { return parsePrimary(); }

ASTNodePtr Parser::parsePrimary() {
    if (check(TokenType::NUMBER)) {
        Token t = consume();
        auto node = std::make_unique<ASTNode>(NodeType::NumberLiteral);
        node->numValue = std::stod(t.value);
        return node;
    }
    if (check(TokenType::TRUE)) {
        consume();
        auto node = std::make_unique<ASTNode>(NodeType::BoolLiteral);
        node->boolValue = true;
        return node;
    }
    if (check(TokenType::FALSE)) {
        consume();
        auto node = std::make_unique<ASTNode>(NodeType::BoolLiteral);
        node->boolValue = false;
        return node;
    }
    if (check(TokenType::IDENTIFIER)) {
        Token t = consume();
        auto node = std::make_unique<ASTNode>(NodeType::Identifier);
        node->strValue = t.value;
        return node;
    }
    if (match(TokenType::LPAREN)) {
        auto expr = parseExpression();
        expect(TokenType::RPAREN, "Expected ')' after grouped expression");
        return expr;
    }
    throw std::runtime_error("Unexpected token '" + peek().value + "' in expression");
}
