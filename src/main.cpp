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

#include "include.h"

const std::string user_config_file = "tmake.tmake";
const std::string system_config_file = "build/tmake.config";
const std::string cache_file = "build/tmake.cache";

static void print_usage() {
    std::cout << "Usage: tmake <command> [options]" << std::endl;
    std::cout << "Commands:" << std::endl;
    std::cout << "  config [args...]  Parse tmake.tmake and generate build/tmake.config" << std::endl;
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


static int run_init() {
    if (std::filesystem::exists(user_config_file)) {
        std::cerr << "Error: " << user_config_file << " already exists." << std::endl;
        return 1;
    }

    std::ofstream config_file(user_config_file);
    if (!config_file.is_open()) {
        std::cerr << "Error: Could not create " << user_config_file << std::endl;
        return 1;
    }
    config_file<<"/*\n";
    config_file<<"tmake configuration file\n";
    config_file<<"\n";
    config_file<<"______________________________ Variables ____________________________\n";
    config_file<<"\n";
    config_file<<"var is used to declare a variable, the value can be a string, a list of strings, or a list of variables\n";
    config_file<<"var variable_name = [\"value1\", \"value2\", \"value3\"]\n";
    config_file<<"var variable_name = \"value\"\n";
    config_file<<"\n";
    config_file<<"\n";
    config_file<<"____________________________ Built in functions ____________________________\n";
    config_file<<"\n";
    config_file<<"PROGRAM is a function that compiles the source files with the specified flags and outputs to the specified directory\n";
    config_file<<"PROGRAM can be used multiple times to create multiple programs with different configurations\n";
    config_file<<"PROGRAM(program_name, files, flags, output_dir, links, include_directories) \n";
    config_file<<"\n";
    config_file<<"COMPILER is a function that sets the compiler to be used can be excluded, default is g++\n";
    config_file<<"COMPILER(compiler) \n";
    config_file<<"\n";
    config_file<<"VERSION is a function that sets the C++ language standard version to be used, can be excluded, default is 17\n";
    config_file<<"VERSION(version)\n";
    config_file<<"\n";
    config_file<<"PRINT is a function that prints a message to the console, can be used for debugging or informational purposes\n";
    config_file<<"PRINT(message)\n";
    config_file<<"\n";
    config_file<<"\n";
    config_file<<"More info: https://github.com/twanofzo1/build_system/tree/main\n";
    config_file<<"*/\n";
    config_file<<"\n";
    config_file<<"\n";
    config_file<<"\n";
    config_file<<"\n";
    config_file<<"// List of source files to be compiled\n";
    config_file<<"// \"src/**.cpp\"   can be used to include all cpp files in the src directory and its subdirectories\n";
    config_file<<"// \"src/*.cpp\"    can be used to include all cpp files in the src directory\n";
    config_file<<"// \"src/test.cpp\" can be used to include a specific file\n";
    config_file<<"var files = [\n";
    config_file<<"    \"src/**.cpp\"\n";
    config_file<<"]\n";
    config_file<<"\n";
    config_file<<"// List of flags for debug mode\n";
    config_file<<"var debug_flags = [\n";
    config_file<<"    \"-Wall\",\n";
    config_file<<"    \"-Wextra\",\n";
    config_file<<"    \"-DDEBUG\"\n";
    config_file<<"]\n";
    config_file<<"\n";
    config_file<<"// List of flags for release mode\n";
    config_file<<"var release_flags = [\n";
    config_file<<"    \"-O3\",\n";
    config_file<<"    \"-DNDEBUG\"\n";
    config_file<<"]\n";
    config_file<<"\n";
    config_file<<"// The name of the output file and the output directory\n";
    config_file<<"var program_name = \"my_program\"\n";
    config_file<<"var output_directory = \"./bin\"\n";
    config_file<<"var links = [\n";
    config_file<<"    \"\"\n";
    config_file<<"]\n";
    config_file<<"var include_directories = [\n";
    config_file<<"    \"src\"\n";
    config_file<<"]\n";
    config_file<<"\n";
    config_file<<"// Set the compiler to be used\n";
    config_file<<"var compiler = \"g++\"\n";
    config_file<<"COMPILER(compiler)\n";
    config_file<<"// Set the C++ language standard version to be used\n";
    config_file<<"VERSION(\"17\")\n";
    config_file<<"LANGUAGE(\"c++\")\n";
    config_file<<"\n";
    config_file<<"// tmake config debug->\n";
    config_file<<"if \"$1\" == \"debug\" {\n";
    config_file<<"    PROGRAM(program_name, files, debug_flags, output_directory, links, include_directories)\n";
    config_file<<"}\n";
    config_file<<"// tmake config release->\n";
    config_file<<"else if \"$1\" == \"release\" {\n";
    config_file<<"    PROGRAM(program_name, files, release_flags, output_directory, links, include_directories)\n";
    config_file<<"}\n";
    config_file<<"// tmake config ... ->\n";
    config_file<<"else {\n";
    config_file<<"    PRINT(\"Invalid build mode. Use 'debug' or 'release'.\")\n";
    config_file<<"}";

    config_file.close();

    // make the build and src directories if they don't exist
    if (!std::filesystem::exists("build")) {
        std::filesystem::create_directory("build");
    }
    if (!std::filesystem::exists("src")) {
        std::filesystem::create_directory("src");
    }
    return 0;
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
    } else if (command == "help" || command == "--help" || command == "-h") {
        print_usage();
        return 0;
    } else if (command == "init") {
        return run_init();
    } else if (command == "clean") {
        std::filesystem::remove_all("build");
        return 0;
    } else {
        std::cerr << "Error: Unknown command '" << command << "'" << std::endl;
        print_usage();
        return 1;
    }

}
