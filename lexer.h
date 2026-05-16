#pragma once
#include <string>
#include <vector>

enum class TokenType {
    // Literals
    NUMBER, BOOLEAN,
    // Identifiers & Keywords
    IDENTIFIER, LET, IF, ELSE, WHILE, PRINT, INPUT, TRUE, FALSE,
    // Operators
    PLUS, MINUS, STAR, SLASH, EQ_EQ, LESS, GREATER, BANG_EQ,
    // Assignment & punctuation
    EQUAL, SEMICOLON, LPAREN, RPAREN, LBRACE, RBRACE,
    // End
    EOF_TOKEN
};

struct Token {
    TokenType type;
    std::string value;
    int line;
};

class Lexer {
public:
    explicit Lexer(const std::string& source);
    std::vector<Token> tokenize();

private:
    std::string src;
    size_t pos;
    int line;

    char peek(int offset = 0);
    char advance();
    void skipWhitespaceAndComments();
    Token readNumber();
    Token readIdentifierOrKeyword();
};
