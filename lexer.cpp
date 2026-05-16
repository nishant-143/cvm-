#include "lexer.h"
#include <stdexcept>
#include <cctype>

Lexer::Lexer(const std::string& source) : src(source), pos(0), line(1) {}

char Lexer::peek(int offset) {
    size_t idx = pos + offset;
    return (idx < src.size()) ? src[idx] : '\0';
}

char Lexer::advance() {
    char c = src[pos++];
    if (c == '\n') line++;
    return c;
}

void Lexer::skipWhitespaceAndComments() {
    while (pos < src.size()) {
        char c = peek();
        if (std::isspace(c)) { advance(); }
        else if (c == '/' && peek(1) == '/') {
            while (pos < src.size() && peek() != '\n') advance();
        } else break;
    }
}

Token Lexer::readNumber() {
    std::string num;
    while (pos < src.size() && (std::isdigit(peek()) || peek() == '.'))
        num += advance();
    return { TokenType::NUMBER, num, line };
}

Token Lexer::readIdentifierOrKeyword() {
    std::string id;
    while (pos < src.size() && (std::isalnum(peek()) || peek() == '_'))
        id += advance();

    if (id == "let")   return { TokenType::LET,   id, line };
    if (id == "if")    return { TokenType::IF,    id, line };
    if (id == "else")  return { TokenType::ELSE,  id, line };
    if (id == "while") return { TokenType::WHILE, id, line };
    if (id == "print") return { TokenType::PRINT, id, line };
    if (id == "input") return { TokenType::INPUT, id, line };
    if (id == "true")  return { TokenType::TRUE,  id, line };
    if (id == "false") return { TokenType::FALSE, id, line };
    return { TokenType::IDENTIFIER, id, line };
}

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;
    while (true) {
        skipWhitespaceAndComments();
        if (pos >= src.size()) { tokens.push_back({ TokenType::EOF_TOKEN, "", line }); break; }

        char c = peek();

        if (std::isdigit(c)) { tokens.push_back(readNumber()); continue; }
        if (std::isalpha(c) || c == '_') { tokens.push_back(readIdentifierOrKeyword()); continue; }

        advance(); // consume the character
        switch (c) {
            case '+': tokens.push_back({ TokenType::PLUS,      "+", line }); break;
            case '-': tokens.push_back({ TokenType::MINUS,     "-", line }); break;
            case '*': tokens.push_back({ TokenType::STAR,      "*", line }); break;
            case '/': tokens.push_back({ TokenType::SLASH,     "/", line }); break;
            case ';': tokens.push_back({ TokenType::SEMICOLON, ";", line }); break;
            case '(': tokens.push_back({ TokenType::LPAREN,    "(", line }); break;
            case ')': tokens.push_back({ TokenType::RPAREN,    ")", line }); break;
            case '{': tokens.push_back({ TokenType::LBRACE,    "{", line }); break;
            case '}': tokens.push_back({ TokenType::RBRACE,    "}", line }); break;
            case '<': tokens.push_back({ TokenType::LESS,      "<", line }); break;
            case '>': tokens.push_back({ TokenType::GREATER,   ">", line }); break;
            case '=':
                if (peek() == '=') { advance(); tokens.push_back({ TokenType::EQ_EQ,  "==", line }); }
                else                             tokens.push_back({ TokenType::EQUAL,  "=",  line });
                break;
            case '!':
                if (peek() == '=') { advance(); tokens.push_back({ TokenType::BANG_EQ,"!=", line }); }
                else throw std::runtime_error("Unexpected '!' at line " + std::to_string(line));
                break;
            default:
                throw std::runtime_error(std::string("Unexpected character '") + c + "' at line " + std::to_string(line));
        }
    }
    return tokens;
}
