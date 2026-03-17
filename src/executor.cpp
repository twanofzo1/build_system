#include "executor.hpp"
#include <filesystem>
#include <algorithm>

namespace fs = std::filesystem;

Executor::Executor(Ast& ast, Arguments& arguments) : ast(ast), arguments(arguments) {}


std::vector<std::string> Executor::expand_glob(const std::string& pattern) {
    if (pattern.find('*') == std::string::npos) {
        return { pattern };
    }

    std::vector<std::string> results;

    size_t last_slash = pattern.rfind('/');
    std::string dir_str = ".";
    std::string glob_part = pattern;
    if (last_slash != std::string::npos) {
        dir_str = pattern.substr(0, last_slash);
        glob_part = pattern.substr(last_slash + 1);
    }

    if (!fs::exists(dir_str) || !fs::is_directory(dir_str)) {
        std::cerr << "Warning: Directory '" << dir_str << "' does not exist for pattern '" << pattern << "'" << std::endl;
        return {};
    }

    std::string extension;
    size_t dot_pos = glob_part.rfind('.');
    if (dot_pos != std::string::npos) {
        extension = glob_part.substr(dot_pos);
    }

    bool recursive = glob_part.find("**") != std::string::npos;

    if (recursive) {
        for (const auto& entry : fs::recursive_directory_iterator(dir_str)) {
            if (entry.is_regular_file()) {
                std::string path = entry.path().string();
                if (extension.empty() || (path.size() >= extension.size()
                    && path.compare(path.size() - extension.size(), extension.size(), extension) == 0)) {
                    results.push_back(path);
                }
            }
        }
    } else {
        for (const auto& entry : fs::directory_iterator(dir_str)) {
            if (entry.is_regular_file()) {
                std::string path = entry.path().string();
                if (extension.empty() || (path.size() >= extension.size()
                    && path.compare(path.size() - extension.size(), extension.size(), extension) == 0)) {
                    results.push_back(path);
                }
            }
        }
    }

    std::sort(results.begin(), results.end());
    return results;
}


std::string Executor::resolve_string(const Ast_index& index) {
    if (index.type == Ast_statement_type::String_literal) {
        std::string value = ast.string_literals[index.index].value;
        if (value.size() >= 2 && value[0] == '$') {
            std::string num_str = value.substr(1);
            int arg_index = std::stoi(num_str);
            int actual_index = arg_index + 1;
            if (actual_index < arguments.size()) {
                return arguments[actual_index];
            }
            return "";
        }
        return value;
    } else if (index.type == Ast_statement_type::Identifier_reference) {
        std::string var_name = ast.identifier_references[index.index].name;
        if (variables.find(var_name) != variables.end() && !variables[var_name].empty()) {
            return variables[var_name][0];
        }
        std::cerr << "Error: Undefined variable '" << var_name << "'" << std::endl;
        return "";
    }
    return "";
}


std::vector<std::string> Executor::resolve_value(const Ast_index& index) {
    if (index.type == Ast_statement_type::String_literal) {
        std::string str = resolve_string(index);
        if (str.find('*') != std::string::npos) {
            return expand_glob(str);
        }
        return { str };
    } else if (index.type == Ast_statement_type::Identifier_reference) {
        std::string var_name = ast.identifier_references[index.index].name;
        if (variables.find(var_name) != variables.end()) {
            return variables[var_name];
        }
        std::cerr << "Error: Undefined variable '" << var_name << "'" << std::endl;
        return {};
    } else if (index.type == Ast_statement_type::Array_literal) {
        std::vector<std::string> values;
        for (const auto& element : ast.array_literals[index.index].elements) {
            std::string str = resolve_string(element);
            if (str.find('*') != std::string::npos) {
                std::vector<std::string> expanded = expand_glob(str);
                values.insert(values.end(), expanded.begin(), expanded.end());
            } else {
                values.push_back(str);
            }
        }
        return values;
    }
    return {};
}


bool Executor::evaluate_condition(const Ast_index& condition) {
    if (condition.type == Ast_statement_type::Binary_expression) {
        const Ast_binary_expression& bin_expr = ast.binary_expressions[condition.index];
        std::string left = resolve_string(bin_expr.left);
        std::string right = resolve_string(bin_expr.right);
        if (bin_expr.op == "==") {
            return left == right;
        } else if (bin_expr.op == "!=") {
            return left != right;
        }
    } else if (condition.type == Ast_statement_type::String_literal) {
        std::string value = resolve_string(condition);
        return value == "true";
    }
    std::cerr << "Error: Unsupported condition type" << std::endl;
    return false;
}


void Executor::execute_variable_declaration(const Ast_index& stmt) {
    Ast_variable_declaration var_decl = ast.variable_declarations[stmt.index];
    std::string var_name = var_decl.name;
    Ast_index var_value = var_decl.value;

    if (var_value.type == Ast_statement_type::String_literal) {
        std::string value = resolve_string(var_value);
        if (value.find('*') != std::string::npos) {
            variables[var_name] = expand_glob(value);
        } else {
            variables[var_name] = {value};
        }
    } else if (var_value.type == Ast_statement_type::Array_literal) {
        std::vector<std::string> values;
        for (const auto& element : ast.array_literals[var_value.index].elements) {
            std::string str = resolve_string(element);
            if (str.find('*') != std::string::npos) {
                std::vector<std::string> expanded = expand_glob(str);
                values.insert(values.end(), expanded.begin(), expanded.end());
            } else {
                values.push_back(str);
            }
        }
        variables[var_name] = values;
    } else {
        variables[var_name] = {};
    }
}


void Executor::execute_if(const Ast_index& stmt) {
    Ast_if_statement if_stmt = ast.if_statements[stmt.index];
    if (evaluate_condition(if_stmt.condition)) {
        execute_stmt(if_stmt.true_block);
    } else if (if_stmt.false_block.type == Ast_statement_type::If) { // else if
        execute_if(if_stmt.false_block);
    } else {
        execute_stmt(if_stmt.false_block);
    }
}


void Executor::handle_compiler(const Ast_function_call& func_call) {
    if (!func_call.arguments.empty()) {
        std::vector<std::string> values = resolve_value(func_call.arguments[0]);
        if (!values.empty()) {
            m_compiler = values[0];
        }
    }
}


void Executor::handle_version(const Ast_function_call& func_call) {
    if (!func_call.arguments.empty()) {
        std::vector<std::string> values = resolve_value(func_call.arguments[0]);
        if (!values.empty()) {
            m_version = values[0];
        }
    }
}

void Executor::handle_language(const Ast_function_call& func_call) {
    if (!func_call.arguments.empty()) {
        std::vector<std::string> values = resolve_value(func_call.arguments[0]);
        if (!values.empty()) {
            std::string lang = values[0];
            if (lang == "c++" || lang == "cpp" || lang == "C++") {
                m_language = "c++";
            } else if (lang == "c" || lang == "C") {
                m_language = "c";
            } else {
                std::cerr << "Warning: Unknown language '" << lang << "', defaulting to c++" << std::endl;
                m_language = "c++";
            }
        }
    }
}

void Executor::handle_print(const Ast_function_call& func_call) {
    if (!func_call.arguments.empty()) {
        std::vector<std::string> values = resolve_value(func_call.arguments[0]);
        for (const auto& val : values) {
            std::cout << "Tmake: " << val << std::endl;
        }
    } else {
        std::cerr << "Error: PRINT function requires at least one argument" << std::endl;
    }
}


void Executor::handle_program(const Ast_function_call& func_call) {
    Build_config prog;
    if (func_call.arguments.size() < 1) {
        std::cerr << "Error: PROGRAM function requires at least one argument" << std::endl;
        return;
    }
    if (func_call.arguments.size() >= 1) {
        std::vector<std::string> name = resolve_value(func_call.arguments[0]);
        if (!name.empty()) {
            prog.program_name = name[0];
        }
    }
    if (func_call.arguments.size() >= 2) {
        prog.files = resolve_value(func_call.arguments[1]);
    }
    if (func_call.arguments.size() >= 3) {
        prog.flags = resolve_value(func_call.arguments[2]);
    }
    if (func_call.arguments.size() >= 4) {
        std::vector<std::string> dir = resolve_value(func_call.arguments[3]);
        if (!dir.empty()) {
            prog.output_directory = dir[0];
        }
    }
    if (func_call.arguments.size() >= 5) {
        for (const auto& link : resolve_value(func_call.arguments[4])) {
            if (!link.empty()) {
                if (link.find(".dll") != std::string::npos || link.find(".so") != std::string::npos) {
                    prog.dynamic_links.push_back(link);
                } else {
                    prog.links.push_back(link);
                }
            }
        }
    }

    m_programs.push_back(prog);
}



void Executor::execute_function_call(const Ast_index& stmt) {
    Ast_function_call func_call = ast.function_calls[stmt.index];

    if (func_call.function_name == "COMPILER") {
        handle_compiler(func_call);
    } else if (func_call.function_name == "VERSION") {
        handle_version(func_call);
    } else if (func_call.function_name == "LANGUAGE") {
        handle_language(func_call);
    } else if (func_call.function_name == "PROGRAM") {
        handle_program(func_call);
    } else {
        std::cerr << "Error: Unknown function '" << func_call.function_name << "'" << std::endl;
    }
}


void Executor::execute_stmt(const Ast_index& stmt_index) {
    if (stmt_index.type == Ast_statement_type::Block) {
        const Ast_block& block = ast.blocks[stmt_index.index];
        for (const auto& stmt : block.statements) {
            execute_stmt(stmt);
        }
        return;
    }

    switch (stmt_index.type) {
        case Ast_statement_type::Variable_declaration:
            execute_variable_declaration(stmt_index);
            break;
        case Ast_statement_type::If:
            execute_if(stmt_index);
            break;
        case Ast_statement_type::Function_call:
            execute_function_call(stmt_index);
            break;
        default:
            std::cerr << "Error: Unsupported statement type " << stmt_type_to_string(stmt_index.type) << std::endl;
            break;
    }
}


void Executor::write_config_file() {
    fs::create_directories("build");
    std::ofstream file("build/tmake.config");
    if (!file.is_open()) {
        std::cerr << "Error: Could not create build/tmake.config" << std::endl;
        return;
    }

    file << "compiler=" << m_compiler << "\n";
    file << "version=" << m_version << "\n";
    file << "language=" << m_language << "\n";
    file << "program_count=" << m_programs.size() << "\n";
    file << "\n";

    for (size_t p = 0; p < m_programs.size(); ++p) {
        const Build_config& prog = m_programs[p];
        file << "[program]\n";
        file << "name=" << prog.program_name << "\n";
        file << "output_directory=" << prog.output_directory << "\n";

        file << "files=";
        for (size_t i = 0; i < prog.files.size(); ++i) {
            file << prog.files[i];
            if (i < prog.files.size() - 1) file << ";";
        }
        file << "\n";

        file << "flags=";
        for (size_t i = 0; i < prog.flags.size(); ++i) {
            file << prog.flags[i];
            if (i < prog.flags.size() - 1) file << ";";
        }
        file << "\n";

        file << "links=";
        for (size_t i = 0; i < prog.links.size(); ++i) {
            file << prog.links[i];
            if (i < prog.links.size() - 1) file << ";";
        }

        file << "\n";

        file << "dynamic_links=";
        for (size_t i = 0; i < prog.dynamic_links.size(); ++i) {
            file << prog.dynamic_links[i];
            if (i < prog.dynamic_links.size() - 1) file << ";";
        }

        file << "\n";
    }

    file.close();
    std::cout << "Configuration written to build/tmake.config (" << m_programs.size() << " program(s))" << std::endl;
}


void Executor::execute() {
    for (const auto& stmt : ast.statements) {
        execute_stmt(stmt);
    }
    if (!m_programs.empty()) {
        write_config_file();
    }
}
