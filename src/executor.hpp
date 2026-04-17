
#pragma once
#include "parser.hpp"
#include "arguments.hpp"
#include <fstream>

struct Build_config
{
    std::string program_name;
    std::string output_directory;
    std::vector<std::string> files;
    std::vector<std::string> flags;
    std::vector<std::string> links;
    std::vector<std::string> include_directories;
    std::vector<std::string> dynamic_links;
};

class Executor
{
private:
    Ast& ast;
    Arguments& arguments;
    std::unordered_map<std::string, std::vector<std::string>> variables;

    std::string m_compiler = "g++";
    std::string m_version = "17";
    std::string m_language = "c++";
    std::vector<Build_config> m_programs;

    void execute_stmt(const Ast_index& stmt_index);
    void execute_variable_declaration(const Ast_index& stmt);
    void execute_if(const Ast_index& stmt);
    void execute_function_call(const Ast_index& stmt);

    std::vector<std::string> resolve_value(const Ast_index& index);
    std::string resolve_string(const Ast_index& index);
    bool evaluate_condition(const Ast_index& condition);

    void handle_compiler(const Ast_function_call& func_call);
    void handle_version(const Ast_function_call& func_call);
    void handle_language(const Ast_function_call& func_call);
    void handle_program(const Ast_function_call& func_call);
    void handle_print(const Ast_function_call& func_call);

    std::vector<std::string> expand_glob(const std::string& pattern);
    void write_config_file();
public:
    Executor(Ast& ast, Arguments& arguments);
    void execute();
};
