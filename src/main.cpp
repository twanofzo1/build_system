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

std::unordered_map<std::string, std::vector<std::string>> variables;
const std::string supported_compilers[] = {"g++"}; 


const std::string user_config_file = "twan_build.txt";
const std::string system_config_file = "build/twan_build.config";
const std::string files_cache_file = "build/twan_build_files.cache";

struct Config
{
    std::vector<std::string> files;
    std::vector<std::string> debug_flags;
    std::vector<std::string> release_flags;
    std::string output_directory = "bin";   // default output directory
    std::string program_name = "program";   // default output name
    std::string compiler = "g++";           // default compiler
    std::string version = "17";             // default C++ standard version

    void load() {
        std::ifstream config_file(user_config_file);
        if (!config_file.is_open()) {
            std::cerr << "Error: Could not open configuration file: " << user_config_file << std::endl;
            return;
        }
        std::string line;
        while (std::getline(config_file, line)) {
            // Parse the configuration file and populate the Config struct
            // This is a placeholder for the actual parsing logic   
        }  
    }

    void save_to_file() {
        std::ofstream config_file(system_config_file);
        if (!config_file.is_open()) {
            std::cerr << "Error: Could not open configuration file for writing: " << system_config_file << std::endl;
            return;
        }
        // Write the configuration to the file
        // This is a placeholder for the actual writing logic
    }   

    void print() const {
        std::cout << "Files: ";
        for (const auto& file : files) {
            std::cout << file << " ";
        }
        std::cout << "\nDebug Flags: ";
        for (const auto& flag : debug_flags) {
            std::cout << flag << " ";
        }
        std::cout << "\nRelease Flags: ";
        for (const auto& flag : release_flags) {
            std::cout << flag << " ";
        }
        std::cout << "\nOutput Directory: " << output_directory;
        std::cout << "\nProgram Name: " << program_name;
        std::cout << "\nCompiler: " << compiler;
        std::cout << "\nC++ Version: " << version;
        std::cout << std::endl;
    }
};













class Arguments{
private:
    std::vector<std::string> args;
public:
    Arguments(int argc, char** argv) {
        for (int i = 0; i < argc; ++i) {
            this->args.push_back(argv[i]);
        }
    }
    int size() const {
        return args.size();
    }
    std::string operator[](int index) const {
        assert(index >= 0 && index < args.size() && "Index out of bounds");
        return args[index];
    }
};





namespace fs = std::filesystem;

int main(int argc, char** argv) {
    Arguments arguments(argc, argv);
    std::ifstream config_file(user_config_file);
    std::string config_content((std::istreambuf_iterator<char>(config_file)), std::istreambuf_iterator<char>());
    Lexer lexer(config_content);

    lexer.lex();
    std::cout << "Tokens:\n";
    for (const auto& token : lexer.get_tokens()) {
        token.print();
    }
    std::cout << std::endl;
    
    Parser parser(lexer.get_tokens());
    Ast ast = parser.parse();
    std::cout << "\nAbstract Syntax Tree:\n";
    ast.print();

    return 0;
}