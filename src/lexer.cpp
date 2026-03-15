
#include "lexer.hpp"
#include "modern_types.h"
#include <string>
#include <unordered_map>
#include <vector>
#include <iostream>





const std::unordered_map<std::string, Token_type> keywords = {
    {"var", Token_type::Var},
    {"if", Token_type::If}, 
    {"else", Token_type::Else}, 
    {"PRINT", Token_type::Print},
    {"PROGRAM", Token_type::Program},
    {"VERSION", Token_type::Version},
    {"COMPILER", Token_type::Compiler},
};



void Token::print() const {
    std::cout << token_type_to_string(type) << ": " << value << std::endl;
}


void Lexer::advance(u8 n) {
    m_pos += n;
    if (m_pos < m_input.size()) {
        m_current_char = m_input[m_pos];
    } else {
        m_current_char = '\0'; // End of input
    }
}

char Lexer::peek(u8 n) {
    if (m_pos + n < m_input.size()) {
        return m_input[m_pos + n];
    } else {
        return '\0'; // End of input
    }
}

Lexer::Lexer(const std::string& input) : m_input(input), m_pos(0), m_line(1) {
    m_current_char = m_input[m_pos];
}

void Lexer::skip_comment() {
    if (peek() == '/') {
        // Single-line comment
        while (m_pos < m_input.size() && m_current_char != '\n') {
            advance();
        }
    } else if (peek() == '*') {
        // Multi-line comment
        m_pos += 2; // Skip the '/*'
        while (m_pos < m_input.size() && !(m_input[m_pos] == '*' && peek() == '/')) {
            m_pos++;
        }
        m_pos += 2; // Skip the '*/'
    } else {
        std::cerr << "Error: Unexpected character after '/': " << peek() << std::endl;
        exit(1);
    }
}

void Lexer::parse_string_literal() {
    std::string str_literal;
    advance(); // Skip the opening quote
    while (m_pos < m_input.size() && m_current_char != '"') {
        str_literal += m_current_char;
        advance();
        
        if (m_current_char == '\0') {
            std::cerr << "Error: Unterminated string literal" << std::endl;
            exit(1);
        }
    }
    advance(); // Skip the closing quote
    tokens.push_back({Token_type::String_literal, str_literal});
}


void Lexer::parse_identifier_or_keyword() {
    std::string identifier;
    while (m_pos < m_input.size() && (isalnum(m_input[m_pos]) || m_input[m_pos] == '_')) {
        identifier += m_input[m_pos];
        advance();
    }
    if (keywords.find(identifier) != keywords.end()) {
        // It's a keyword
        tokens.push_back({keywords.at(identifier), identifier});
    } else {
        // It's an identifier
        tokens.push_back({Token_type::Identifier, identifier});
    }
}

void Lexer::lex(){
    while (m_pos < m_input.size())
    {
        switch (m_current_char)
        {
        case ' ':
        case '\t':
        case '\r':
            advance();
            continue;
        case '\n':
            m_line++;
            advance();
            continue;
        
        case '(':
            tokens.push_back({Token_type::LParen, "("});
            advance();
            continue;
        case ')':
            tokens.push_back({Token_type::RParen, ")"});
            advance();
            continue;
        case '{':
            tokens.push_back({Token_type::LBrace, "{"});
            advance();
            continue;
        case '}':
            tokens.push_back({Token_type::RBrace, "}"});
            advance();
            continue;
        case ',':
            tokens.push_back({Token_type::Comma, ","});
            advance();
            continue;
        case '=':
            if (peek() == '=') {
                tokens.push_back({Token_type::Equal, "=="});
                advance(2);
            } else {
                tokens.push_back({Token_type::Assign, "="});
                advance();
            }
            continue;
        case '!':
            if (peek() == '=') {
                tokens.push_back({Token_type::Not_equal, "!="});
                advance(2);
            } else {
                tokens.push_back({Token_type::Not, "!"});
                advance();
            }
            continue;
        case '&':
            if (peek() == '&') {
                tokens.push_back({Token_type::And, "&&"});
                advance(2);
            } else {
                std::cerr << "Error: Unexpected character after '&': " << peek() << std::endl;
                exit(1);
            }
            continue;
        case '|':
            if (peek() == '|') {
                tokens.push_back({Token_type::Or, "||"});
                advance(2);
            } else {
                std::cerr << "Error: Unexpected character after '|': " << peek() << std::endl;
                exit(1);
            }
            continue;

        case '/':
            skip_comment();
            continue;
        case '"':
            parse_string_literal();
            continue;
        
        default:
            parse_identifier_or_keyword();
            continue;

        case '\0':
            return;
        }
    }
    
}
    
const std::vector<Token>& Lexer::get_tokens() const {
    return tokens;
}