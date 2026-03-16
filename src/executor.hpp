
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
};

class Executor
{
private:
    Ast& ast;
    Arguments& arguments;
    std::unordered_map<std::string, std::vector<std::string>> variables;

    std::string compiler = "g++";
    std::string version = "17";
    std::vector<Build_config> programs;

    void execute_stmt(const Ast_index& stmt_index);
    void execute_variable_declaration(const Ast_index& stmt);
    void execute_print(const Ast_index& stmt);
    void execute_if(const Ast_index& stmt);
    void execute_function_call(const Ast_index& stmt);

    std::vector<std::string> resolve_value(const Ast_index& index);
    std::string resolve_string(const Ast_index& index);
    bool evaluate_condition(const Ast_index& condition);

    void handle_compiler(const Ast_function_call& func_call);
    void handle_version(const Ast_function_call& func_call);
    void handle_program(const Ast_function_call& func_call);

    std::vector<std::string> expand_glob(const std::string& pattern);
    void write_config_file();
public:
    Executor(Ast& ast, Arguments& arguments);
    void execute();
};
