#include "lexer.h"
#include "parser.h"
#include "ast.h"
#include "compiler.h"
#include "vm.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

// ─────────────────────────────────────────────────────────────────────────────
static void runSource(const std::string& source, bool debugAST, bool debugBC) {
    try {
        // 1. Lex
        Lexer lexer(source);
        auto tokens = lexer.tokenize();

        // 2. Parse → AST
        Parser parser(tokens);
        auto ast = parser.parse();

        if (debugAST) {
            std::cout << "\n=== Abstract Syntax Tree ===\n";
            printAST(ast.get());
            std::cout << "============================\n";
        }

        // 3. Compile → Bytecode
        Compiler compiler;
        Chunk chunk = compiler.compile(ast.get());

        if (debugBC) chunk.disassemble();

        // 4. Execute in VM
        VM vm;
        vm.execute(chunk);

    } catch (const std::exception& e) {
        std::cerr << "[ERROR] " << e.what() << "\n";
    }
}

// ─────────────────────────────────────────────────────────────────────────────
static void repl() {
    std::cout << "CVM++ REPL  (type 'exit' to quit, '--ast' / '--bc' for debug, 'reset' to clear)\n";
    std::cout << "----------------------------------------------------------------------------------\n";
    bool debugAST = false, debugBC = false;
    std::string session; // accumulated source across all lines

    while (true) {
        std::cout << "cvm> ";
        std::string line;
        if (!std::getline(std::cin, line)) break;
        if (line == "exit" || line == "quit") break;
        if (line == "reset") { session.clear(); std::cout << "Session cleared.\n"; continue; }
        if (line == "--ast") { debugAST = !debugAST; std::cout << "AST debug " << (debugAST?"ON":"OFF") << "\n"; continue; }
        if (line == "--bc")  { debugBC  = !debugBC;  std::cout << "Bytecode debug " << (debugBC?"ON":"OFF") << "\n"; continue; }
        if (line.empty()) continue;

        // Append new line to the full session source and re-run everything
        std::string candidate = session + line + "\n";
        try {
            Lexer lexer(candidate);
            auto tokens = lexer.tokenize();
            Parser parser(tokens);
            auto ast = parser.parse();

            if (debugAST) {
                std::cout << "\n=== Abstract Syntax Tree ===\n";
                printAST(ast.get());
                std::cout << "============================\n";
            }

            Compiler compiler;
            Chunk chunk = compiler.compile(ast.get());

            if (debugBC) chunk.disassemble();

            VM vm;
            vm.execute(chunk);

            session = candidate; // commit line only on success
        } catch (const std::exception& e) {
            std::cerr << "[ERROR] " << e.what() << "\n";
            // session unchanged — bad line is discarded
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
int main(int argc, char* argv[]) {
    bool debugAST = false, debugBC = false;
    std::string filename;

    for (int i = 1; i < argc; i++) {
        std::string arg(argv[i]);
        if (arg == "--ast") debugAST = true;
        else if (arg == "--bc") debugBC = true;
        else filename = arg;
    }

    if (filename.empty()) {
        repl();
    } else {
        std::ifstream file(filename);
        if (!file) { std::cerr << "Cannot open file: " << filename << "\n"; return 1; }
        std::ostringstream buf;
        buf << file.rdbuf();
        runSource(buf.str(), debugAST, debugBC);
    }
    return 0;
}