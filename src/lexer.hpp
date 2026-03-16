#pragma once



#include "lexer.hpp"
#include "modern_types.h"
#include <string>
#include <unordered_map>
#include <vector>
#include <iostream>

enum class Token_type {
    Identifier,
    Var,
    If,
    Else,
    LParen,
    RParen,
    LBrace,
    RBrace,
    LBracket,
    RBracket,
    Comma,
    Assign,
    Equal,
    Not_equal,
    And,
    Or,
    Not,
    Program,
    Version,
    Compiler,
    String_literal,
    Print
};

inline std::string token_type_to_string(Token_type type) {
    switch (type) {
        case Token_type::Identifier: return "Identifier";
        case Token_type::Var: return "Var";
        case Token_type::If: return "If";
        case Token_type::Else: return "Else";
        case Token_type::LParen: return "LParen";
        case Token_type::RParen: return "RParen";
        case Token_type::LBrace: return "LBrace";
        case Token_type::RBrace: return "RBrace";
        case Token_type::LBracket: return "LBracket";
        case Token_type::RBracket: return "RBracket";
        case Token_type::Comma: return "Comma";
        case Token_type::Assign: return "Assign";
        case Token_type::Equal: return "Equal";
        case Token_type::Not_equal: return "Not equal";
        case Token_type::And: return "And";
        case Token_type::Or: return "Or";
        case Token_type::Not: return "Not";
        case Token_type::Program: return "Program";
        case Token_type::Version: return "Version";
        case Token_type::Compiler: return "Compiler";
        case Token_type::String_literal: return "String literal";
        case Token_type::Print: return "Print";
        default: return "Unknown";
    }
}



struct Token {
    Token_type type;
    std::string value;
    void print() const;
};

class Lexer {
    const std::string& m_input;
    u64 m_pos;
    char m_current_char;
    u64 m_line;
    std::vector<Token> tokens;

    void advance(u8 n = 1);
    char peek(u8 n = 1);
    void skip_comment();
    void parse_string_literal();
    void parse_identifier_or_keyword();
public:
    Lexer(const std::string& input);
    void lex();
    std::vector<Token>& get_tokens();
};