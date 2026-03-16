#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <filesystem>
#include <unordered_map>
#include <cassert>

#include "modern_types.h"
#include "printf_color.h"
#include "lexer.hpp"
#include "parser.hpp"
#include "arguments.hpp"
#include "executor.hpp"
#include "builder.hpp"

const std::string user_config_file = "tmake.txt";
const std::string system_config_file = "build/tmake.config";
const std::string cache_file = "build/tmake.cache";

static void print_usage() {
    std::cout << "Usage: tmake <command> [options]" << std::endl;
    std::cout << "Commands:" << std::endl;
    std::cout << "  config [args...]  Parse tmake.txt and generate build/tmake.config" << std::endl;
    std::cout << "  build             Read build/tmake.config and compile all targets" << std::endl;
}

static int run_config(Arguments& arguments) {
    std::ifstream config_file(user_config_file);
    if (!config_file.is_open()) {
        std::cerr << "Error: Could not open " << user_config_file << std::endl;
        return 1;
    }

    std::string config_content((std::istreambuf_iterator<char>(config_file)), std::istreambuf_iterator<char>());
    config_file.close();

    Lexer lexer(config_content);
    lexer.lex();

    Parser parser(lexer.get_tokens());
    Ast ast = parser.parse();

    Executor executor(ast, arguments);
    executor.execute();

    return 0;
}

static int run_build() {
    Builder builder(system_config_file, cache_file);
    return builder.build() ? 0 : 1;
}

int main(int argc, char** argv) {
    Arguments arguments(argc, argv);

    if (arguments.size() < 2) {
        print_usage();
        return 1;
    }

    std::string command = arguments[1];

    if (command == "config") {
        return run_config(arguments);
    } else if (command == "build") {
        return run_build();
    } else {
        std::cerr << "Error: Unknown command '" << command << "'" << std::endl;
        print_usage();
        return 1;
    }
}
