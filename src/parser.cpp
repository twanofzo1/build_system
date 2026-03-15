#include "parser.hpp"

Parser::Parser(const std::vector<Token>& tokens)
    : m_tokens(tokens), m_pos(0) {}

Token Parser::peek(u64 offset) const {
    if (m_pos + offset < m_tokens.size()) {
        return m_tokens[m_pos + offset];
    } else {
        return {Token_type::Identifier, ""}; // Return a default token if out of bounds
    }
}

Token Parser::consume(Token_type expected_type) {
    if (m_pos < m_tokens.size() && m_tokens[m_pos].type == expected_type) {
        return m_tokens[m_pos++];
    } else {
        std::cerr << "Error: Expected token of type " << token_type_to_string(expected_type)
                  << " but got " << token_type_to_string(peek().type) << std::endl;
        exit(1);
    }
}

const Ast& Parser::parse() {
    while (m_pos < m_tokens.size())
    {
        Ast_index index = parse_statement();
        m_ast.statements.push_back(index); // pushback global statement index
    }
    
    return m_ast;
}

Ast_index Parser::parse_statement() {
    Token current = peek();
    Ast_index result;

    switch (current.type) {
        case Token_type::Var:
            return parse_variable_declaration();

        case Token_type::If:
            return parse_if_statement();
        case Token_type::Print:
            return parse_print_statement();
        case Token_type::String_literal:
            result = parse_string_literal();
            break;
        case Token_type::Identifier:
            result = parse_identifier_reference();
            break;
        case Token_type::LBrace:
            // Distinguish array literal { val, val } from code block { stmt stmt }
            if (peek(1).type == Token_type::RBrace || peek(2).type == Token_type::Comma) {
                return parse_array_literal();
            }
            return parse_block();
        case Token_type::Compiler:
        case Token_type::Program:
        case Token_type::Version:
            return parse_function_call();
        default:
            std::cerr << "Error: Unexpected token " << current.value << std::endl;
            m_pos++; // Skip the unexpected token to avoid infinite loop
            return {Ast_statement_type::Invalid, 0};
    }

    // After value-like expressions, check for binary comparison operators
    if (peek().type == Token_type::Equal || peek().type == Token_type::Not_equal) {
        Token op_token = m_tokens[m_pos++];
        Ast_index right = parse_statement();

        Ast_binary_expression bin_expr;
        bin_expr.left = result;
        bin_expr.op = op_token.value;
        bin_expr.right = right;

        m_ast.binary_expressions.push_back(bin_expr);
        return {Ast_statement_type::Binary_expression, static_cast<u32>(m_ast.binary_expressions.size() - 1)};
    }

    return result;
}

Ast_index Parser::parse_variable_declaration() {
    consume(Token_type::Var);
    Token name_token = consume(Token_type::Identifier);
    consume(Token_type::Assign);
    Ast_index value_index = parse_statement();

    Ast_variable_declaration var_decl;
    var_decl.name = name_token.value;
    var_decl.value = value_index;

    m_ast.variable_declarations.push_back(var_decl);
    return {Ast_statement_type::Variable_declaration, static_cast<u32>(m_ast.variable_declarations.size() - 1)};
}

Ast_index Parser::parse_if_statement() {
    consume(Token_type::If);
    Ast_index condition_index = parse_statement(); // Parses e.g. "$1" == "debug" as binary expr

    Ast_index true_block = parse_block();

    Ast_index false_block = {Ast_statement_type::Invalid, 0};
    if (peek().type == Token_type::Else) {
        consume(Token_type::Else);
        if (peek().type == Token_type::If) {
            false_block = parse_if_statement(); // else if
        } else {
            false_block = parse_block();
        }
    }

    Ast_if_statement if_stmt;
    if_stmt.condition = condition_index;
    if_stmt.true_block = true_block;
    if_stmt.false_block = false_block;

    m_ast.if_statements.push_back(if_stmt);
    return {Ast_statement_type::If, static_cast<u32>(m_ast.if_statements.size() - 1)};
}


Ast_index Parser::parse_print_statement() {
    consume(Token_type::Print);
    consume(Token_type::LParen);
    Ast_index value_index = parse_statement();
    consume(Token_type::RParen);

    Ast_print_statement print_stmt;
    print_stmt.value = value_index;

    m_ast.print_statements.push_back(print_stmt);
    return {Ast_statement_type::Print, static_cast<u32>(m_ast.print_statements.size() - 1)};
}


Ast_index Parser::parse_string_literal() {
    Token string_token = consume(Token_type::String_literal);

    Ast_string_literal str_lit;
    str_lit.value = string_token.value;

    m_ast.string_literals.push_back(str_lit);
    return {Ast_statement_type::String_literal, static_cast<u32>(m_ast.string_literals.size() - 1)};
}


Ast_index Parser::parse_block() {
    consume(Token_type::LBrace);
    Ast_block block;

    while (peek().type != Token_type::RBrace) {
        block.statements.push_back(parse_statement());
    }
    consume(Token_type::RBrace);
    m_ast.blocks.push_back(block);
    return {Ast_statement_type::Block, static_cast<u32>(m_ast.blocks.size() - 1)};
}


Ast_index Parser::parse_array_literal() {
    consume(Token_type::LBrace);
    Ast_array_literal arr;

    while (peek().type != Token_type::RBrace) {
        arr.elements.push_back(parse_statement());
        if (peek().type == Token_type::Comma) {
            consume(Token_type::Comma);
        }
    }
    consume(Token_type::RBrace);

    m_ast.array_literals.push_back(arr);
    return {Ast_statement_type::Array_literal, static_cast<u32>(m_ast.array_literals.size() - 1)};
}


Ast_index Parser::parse_identifier_reference() {
    Token id_token = consume(Token_type::Identifier);

    Ast_identifier_reference id_ref;
    id_ref.name = id_token.value;

    m_ast.identifier_references.push_back(id_ref);
    return {Ast_statement_type::Identifier_reference, static_cast<u32>(m_ast.identifier_references.size() - 1)};
}


Ast_index Parser::parse_function_call() {
    Token function_name_token = m_tokens[m_pos++]; // Accept any keyword token (Compiler, Program, Version)
    consume(Token_type::LParen);

    std::vector<Ast_index> arguments;
    if (peek().type != Token_type::RParen) {
        do {
            arguments.push_back(parse_statement());
        } while (peek().type == Token_type::Comma && consume(Token_type::Comma).type == Token_type::Comma);
    }
    consume(Token_type::RParen);

    Ast_function_call func_call;
    func_call.function_name = function_name_token.value;
    func_call.arguments = arguments;

    m_ast.function_calls.push_back(func_call);
    return {Ast_statement_type::Function_call, static_cast<u32>(m_ast.function_calls.size() - 1)};
}


void Ast_variable_declaration::print() const {
    std::cout << "Variable Declaration: " << name << " = ";
    value.print();
}

void Ast_if_statement::print() const {
    std::cout << "If Statement:\nCondition: ";
    condition.print();
    std::cout << "\nTrue Block:\n";
    true_block.print();
    if (false_block.index != 0) { 
        std::cout << "\nFalse Block:\n";
        false_block.print();
    }
}

void Ast_print_statement::print() const {
    std::cout << "Print Statement: ";
    value.print();
}

void Ast_string_literal::print() const {
    std::cout << "String Literal: \"" << value << "\"";
}

void Ast_block::print() const {
    std::cout << "Block:\n";
    for (const auto& stmt : statements) {
        stmt.print();
        std::cout << std::endl;
    }
}

void Ast_function_call::print() const {
    std::cout << "Function Call: " << function_name << "(";
    for (size_t i = 0; i < arguments.size(); ++i) {
        arguments[i].print();
        if (i < arguments.size() - 1) {
            std::cout << ", ";
        }
    }
    std::cout << ")";
}

void Ast_array_literal::print() const {
    std::cout << "Array: {";
    for (size_t i = 0; i < elements.size(); ++i) {
        elements[i].print();
        if (i < elements.size() - 1) {
            std::cout << ", ";
        }
    }
    std::cout << "}";
}

void Ast_binary_expression::print() const {
    std::cout << "Binary Expression: (";
    left.print();
    std::cout << " " << op << " ";
    right.print();
    std::cout << ")";
}

void Ast_identifier_reference::print() const {
    std::cout << "Identifier: " << name;
}

void Ast::print() const {
    std::cout << "AST:\n";
    for (const auto& var_decl : variable_declarations) {
        var_decl.print();
        std::cout << std::endl;
    }
    for (const auto& if_stmt : if_statements) {
        if_stmt.print();
        std::cout << std::endl;
    }
    for (const auto& print_stmt : print_statements) {
        print_stmt.print();
        std::cout << std::endl;
    }
    for (const auto& str_lit : string_literals) {
        str_lit.print();
        std::cout << std::endl;
    }
    for (const auto& block : blocks) {
        block.print();
        std::cout << std::endl;
    }
    for (const auto& func_call : function_calls) {
        func_call.print();
        std::cout << std::endl;
    }
    for (const auto& arr_lit : array_literals) {
        arr_lit.print();
        std::cout << std::endl;
    }
    for (const auto& bin_expr : binary_expressions) {
        bin_expr.print();
        std::cout << std::endl;
    }
    for (const auto& id_ref : identifier_references) {
        id_ref.print();
        std::cout << std::endl;
    }
}


void Ast_index::print() const {
    switch (type) {
        case Ast_statement_type::Variable_declaration: 
            std::cout << "Variable declaration at index " << index;
            break;
        case Ast_statement_type::If: 
            std::cout << "If statement at index " << index;
            break;
        case Ast_statement_type::Block: 
            std::cout << "Block at index " << index;
            break;
        case Ast_statement_type::Print: 
            std::cout << "Print statement at index " << index;
            break;
        case Ast_statement_type::String_literal: 
            std::cout << "String literal at index " << index;
            break;
        case Ast_statement_type::Function_call:
            std::cout << "Function call at index " << index;
            break;
        case Ast_statement_type::Array_literal:
            std::cout << "Array literal at index " << index;
            break;
        case Ast_statement_type::Binary_expression:
            std::cout << "Binary expression at index " << index;
            break;
        case Ast_statement_type::Identifier_reference:
            std::cout << "Identifier reference at index " << index;
            break;
        default: 
            std::cout << "Unknown AST node type";
    }
}