#pragma once
#include "lexer.hpp"

enum class Ast_statement_type {
    Variable_declaration,
    If,
    Block,
    Print,
    String_literal,
    Function_call,
    Array_literal,
    Binary_expression,
    Identifier_reference,
    Invalid,
};

inline std::string stmt_type_to_string(Ast_statement_type type) {
    switch (type) {
        case Ast_statement_type::Variable_declaration: return "Variable declaration";
        case Ast_statement_type::If: return "If";
        case Ast_statement_type::Block: return "Block";
        case Ast_statement_type::Print: return "Print";
        case Ast_statement_type::String_literal: return "String literal";
        case Ast_statement_type::Function_call: return "Function call";
        case Ast_statement_type::Array_literal: return "Array literal";
        case Ast_statement_type::Binary_expression: return "Binary expression";
        case Ast_statement_type::Identifier_reference: return "Identifier reference";
        case Ast_statement_type::Invalid: return "Invalid";
        default: return "Unknown";
    }
}

struct Ast_index
{
    Ast_statement_type type;
    u32 index;
    void print() const;
};

struct Ast_variable_declaration
{
    std::string name;
    Ast_index value;
    void print() const;
};

struct Ast_block
{
    std::vector<Ast_index> statements;
    void print() const;
};

struct Ast_if_statement
{
    Ast_index condition;    
    Ast_index true_block;
    Ast_index false_block;
    void print() const;
};

struct Ast_print_statement
{
    Ast_index value;
    void print() const;
};

struct Ast_string_literal
{
    std::string value;
    void print() const;
};

struct Ast_function_call
{
    std::string function_name;
    std::vector<Ast_index> arguments;
    void print() const;
};

struct Ast_array_literal
{
    std::vector<Ast_index> elements;
    void print() const;
};

struct Ast_binary_expression
{
    Ast_index left;
    std::string op;
    Ast_index right;
    void print() const;
};

struct Ast_identifier_reference
{
    std::string name;
    void print() const;
};


struct Ast{
    std::vector<Ast_variable_declaration> variable_declarations;
    std::vector<Ast_if_statement> if_statements;
    std::vector<Ast_print_statement> print_statements;
    std::vector<Ast_string_literal> string_literals;
    std::vector<Ast_block> blocks;
    std::vector<Ast_index> statements;
    std::vector<Ast_function_call> function_calls;
    std::vector<Ast_array_literal> array_literals;
    std::vector<Ast_binary_expression> binary_expressions;
    std::vector<Ast_identifier_reference> identifier_references;

    void print() const;
};


class Parser
{
private:
    u64 m_pos;
    std::vector<Token>& m_tokens;
    Ast m_ast;
    Token m_current_token;

    Token peek(u64 offset = 0) const;
    Token consume(Token_type expected_type);

    Ast_index parse_variable_declaration();
    Ast_index parse_if_statement();
    Ast_index parse_print_statement();
    Ast_index parse_string_literal();
    Ast_index parse_block();
    Ast_index parse_function_call();
    Ast_index parse_array_literal();
    Ast_index parse_identifier_reference();
    Ast_index parse_statement();
public:
    Parser(std::vector<Token>& tokens);
    const Ast& parse();
};

