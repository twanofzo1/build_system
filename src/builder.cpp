#include "builder.hpp"
#include <fstream>
#include <iostream>
#include <filesystem>
#include <cstdlib>
#include <sstream>
#include <array>

namespace fs = std::filesystem;

Builder::Builder(const std::string& config_path, const std::string& cache_path)
    : config_path(config_path), cache_path(cache_path) {}


std::vector<std::string> Builder::split(const std::string& str, char delimiter) {
    std::vector<std::string> result;
    if (str.empty()) return result;

    size_t start = 0;
    size_t end = str.find(delimiter);
    while (end != std::string::npos) {
        std::string token = str.substr(start, end - start);
        if (!token.empty()) {
            result.push_back(token);
        }
        start = end + 1;
        end = str.find(delimiter, start);
    }
    std::string last = str.substr(start);
    if (!last.empty()) {
        result.push_back(last);
    }
    return result;
}


long long Builder::get_mtime(const std::string& path) {
    if (!fs::exists(path)) return 0;
    auto ftime = fs::last_write_time(path);
    auto duration = ftime.time_since_epoch();
    return std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
}


bool Builder::parse_config() {
    std::ifstream file(config_path);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open " << config_path << std::endl;
        std::cerr << "Run 'tmake config' first to generate the config file." << std::endl;
        return false;
    }

    std::string line;
    Build_target* current_target = nullptr;

    while (std::getline(file, line)) {
        if (line.empty()) continue;

        if (line == "[program]") {
            targets.emplace_back();
            current_target = &targets.back();
            continue;
        }

        size_t eq = line.find('=');
        if (eq == std::string::npos) continue;

        std::string key = line.substr(0, eq);
        std::string value = line.substr(eq + 1);

        if (key == "compiler") {
            compiler = value;
        } else if (key == "version") {
            version = value;
        } else if (key == "program_count") {
            // informational
        } else if (current_target != nullptr) {
            if (key == "name") {
                current_target->name = value;
            } else if (key == "output_directory") {
                current_target->output_directory = value;
            } else if (key == "files") {
                current_target->files = split(value, ';');
            } else if (key == "flags") {
                current_target->flags = split(value, ';');
            }
        }
    }

    file.close();
    return true;
}


// Cache format:
// [file]
// source=path/to/file.cpp
// mtime=123456789
// deps=header1.hpp;header2.hpp
// dep_mtimes=123456789;123456789
void Builder::load_cache() {
    std::ifstream file(cache_path);
    if (!file.is_open()) return;

    std::string line;
    Cache_entry* current = nullptr;

    while (std::getline(file, line)) {
        if (line.empty()) continue;

        if (line == "[file]") {
            // will set current once we read the source= line
            current = nullptr;
            continue;
        }

        size_t eq = line.find('=');
        if (eq == std::string::npos) continue;

        std::string key = line.substr(0, eq);
        std::string value = line.substr(eq + 1);

        if (key == "source") {
            cache[value] = Cache_entry{};
            cache[value].source_file = value;
            current = &cache[value];
        } else if (current != nullptr) {
            if (key == "mtime") {
                current->last_modified = std::stoll(value);
            } else if (key == "deps") {
                current->dependencies = split(value, ';');
            } else if (key == "dep_mtimes") {
                std::vector<std::string> parts = split(value, ';');
                for (const auto& p : parts) {
                    current->dep_modified.push_back(std::stoll(p));
                }
            }
        }
    }

    file.close();
}


void Builder::save_cache() {
    fs::create_directories(fs::path(cache_path).parent_path());
    std::ofstream file(cache_path);
    if (!file.is_open()) {
        std::cerr << "Warning: Could not write cache file " << cache_path << std::endl;
        return;
    }

    for (const auto& [source, entry] : cache) {
        file << "[file]\n";
        file << "source=" << entry.source_file << "\n";
        file << "mtime=" << entry.last_modified << "\n";

        file << "deps=";
        for (size_t i = 0; i < entry.dependencies.size(); ++i) {
            file << entry.dependencies[i];
            if (i < entry.dependencies.size() - 1) file << ";";
        }
        file << "\n";

        file << "dep_mtimes=";
        for (size_t i = 0; i < entry.dep_modified.size(); ++i) {
            file << entry.dep_modified[i];
            if (i < entry.dep_modified.size() - 1) file << ";";
        }
        file << "\n\n";
    }

    file.close();
}


std::vector<std::string> Builder::get_dependencies(const std::string& source, const Build_target& target) {
    // Use compiler -MM to get header dependencies
    std::string cmd = compiler + " -std=c++" + version;
    for (const auto& flag : target.flags) {
        cmd += " " + flag;
    }
    cmd += " -MM " + source + " 2>/dev/null";

    std::array<char, 4096> buffer;
    std::string output;

    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) return {};

    while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
        output += buffer.data();
    }
    pclose(pipe);

    // Parse the output: "target.o: source.cpp dep1.hpp dep2.hpp ..."
    // Lines may be continued with backslash
    // Remove backslash-newlines
    std::string cleaned;
    for (size_t i = 0; i < output.size(); ++i) {
        if (output[i] == '\\' && i + 1 < output.size() && output[i + 1] == '\n') {
            i++; // skip both backslash and newline
            continue;
        }
        cleaned += output[i];
    }

    // Find the colon, everything after it is the dependency list
    size_t colon = cleaned.find(':');
    if (colon == std::string::npos) return {};

    std::string dep_str = cleaned.substr(colon + 1);

    // Split by whitespace
    std::vector<std::string> deps;
    std::istringstream iss(dep_str);
    std::string token;
    while (iss >> token) {
        // Skip the source file itself
        if (token != source) {
            deps.push_back(token);
        }
    }

    return deps;
}


bool Builder::needs_rebuild(const std::string& source, const std::vector<std::string>& deps) {
    auto it = cache.find(source);
    if (it == cache.end()) {
        return true; // not in cache, needs build
    }

    const Cache_entry& entry = it->second;

    // Check if source file itself changed
    long long current_mtime = get_mtime(source);
    if (current_mtime != entry.last_modified) {
        return true;
    }

    // Check if dependency list changed
    if (deps.size() != entry.dependencies.size()) {
        return true;
    }
    for (size_t i = 0; i < deps.size(); ++i) {
        if (deps[i] != entry.dependencies[i]) {
            return true;
        }
    }

    // Check if any dependency was modified
    if (entry.dep_modified.size() != deps.size()) {
        return true;
    }
    for (size_t i = 0; i < deps.size(); ++i) {
        long long dep_mtime = get_mtime(deps[i]);
        if (dep_mtime != entry.dep_modified[i]) {
            return true;
        }
    }

    return false;
}


std::string Builder::get_object_path(const std::string& source, const Build_target& target) {
    // build/obj/<target_name>/<source_with_slashes_replaced>.o
    fs::path obj_dir = fs::path("build") / "obj" / target.name;
    // Replace path separators with underscores to flatten
    std::string flat = source;
    for (auto& c : flat) {
        if (c == '/' || c == '\\') c = '_';
    }
    // Replace .cpp with .o
    size_t dot = flat.rfind('.');
    if (dot != std::string::npos) {
        flat = flat.substr(0, dot) + ".o";
    }
    return (obj_dir / flat).string();
}


bool Builder::compile_file(const std::string& source, const std::string& object, const Build_target& target) {
    // Create object directory
    fs::create_directories(fs::path(object).parent_path());

    // compiler -std=c++VERSION flags... -c source -o object
    std::string cmd = compiler + " -std=c++" + version;
    for (const auto& flag : target.flags) {
        cmd += " " + flag;
    }
    cmd += " -c " + source + " -o " + object;

    std::cout << "  CC " << source << std::endl;

    int result = std::system(cmd.c_str());
    if (result != 0) {
        std::cerr << "  -> FAILED: " << source << std::endl;
        return false;
    }
    return true;
}


bool Builder::link_target(const std::vector<std::string>& objects, const Build_target& target) {
    if (!target.output_directory.empty()) {
        fs::create_directories(target.output_directory);
    }

    std::string output_path;
    if (!target.output_directory.empty()) {
        output_path = target.output_directory;
        if (output_path.back() != '/') {
            output_path += '/';
        }
        output_path += target.name;
    } else {
        output_path = target.name;
    }

    // compiler flags... objects... -o output
    std::string cmd = compiler + " -std=c++" + version;
    for (const auto& flag : target.flags) {
        cmd += " " + flag;
    }
    for (const auto& obj : objects) {
        cmd += " " + obj;
    }
    cmd += " -o " + output_path;

    std::cout << "  LINK " << output_path << std::endl;

    int result = std::system(cmd.c_str());
    if (result != 0) {
        std::cerr << "  -> LINK FAILED" << std::endl;
        return false;
    }
    return true;
}


bool Builder::build_target(const Build_target& target) {
    if (target.files.empty()) {
        std::cerr << "Warning: No source files for target '" << target.name << "', skipping." << std::endl;
        return true;
    }

    std::cout << "[BUILD] " << target.name << std::endl;

    // Validate that all source files exist
    for (const auto& file : target.files) {
        if (!fs::exists(file)) {
            std::cerr << "  Error: Source file '" << file << "' does not exist." << std::endl;
            return false;
        }
    }

    std::vector<std::string> objects;
    int compiled = 0;
    int skipped = 0;

    for (const auto& source : target.files) {
        std::string object = get_object_path(source, target);
        objects.push_back(object);

        // Get dependency list for this source
        std::vector<std::string> deps = get_dependencies(source, target);

        bool obj_exists = fs::exists(object);
        if (obj_exists && !needs_rebuild(source, deps)) {
            skipped++;
            continue;
        }

        // Compile this file
        if (!compile_file(source, object, target)) {
            return false;
        }
        compiled++;

        // Update cache entry
        Cache_entry entry;
        entry.source_file = source;
        entry.last_modified = get_mtime(source);
        entry.dependencies = deps;
        for (const auto& dep : deps) {
            entry.dep_modified.push_back(get_mtime(dep));
        }
        cache[source] = entry;
    }

    if (compiled == 0 && skipped == (int)target.files.size()) {
        // Check if the final binary exists
        std::string output_path;
        if (!target.output_directory.empty()) {
            output_path = target.output_directory;
            if (output_path.back() != '/') output_path += '/';
            output_path += target.name;
        } else {
            output_path = target.name;
        }

        if (fs::exists(output_path)) {
            std::cout << "  Up to date. (" << skipped << " file(s) unchanged)" << std::endl;
            return true;
        }
        // Binary missing, need to re-link
    }

    std::cout << "  " << compiled << " compiled, " << skipped << " up to date" << std::endl;

    // Link all object files
    if (!link_target(objects, target)) {
        return false;
    }

    std::cout << "  -> OK" << std::endl;
    return true;
}


bool Builder::build() {
    if (!parse_config()) {
        return false;
    }

    if (targets.empty()) {
        std::cerr << "Error: No programs defined in config." << std::endl;
        return false;
    }

    load_cache();

    std::cout << "Building " << targets.size() << " target(s) with " << compiler << " (C++" << version << ")" << std::endl;
    std::cout << std::endl;

    bool all_ok = true;
    for (const auto& target : targets) {
        if (!build_target(target)) {
            all_ok = false;
        }
        std::cout << std::endl;
    }

    save_cache();

    if (all_ok) {
        std::cout << "Build complete: " << targets.size() << "/" << targets.size() << " succeeded." << std::endl;
    } else {
        std::cerr << "Build finished with errors." << std::endl;
    }

    return all_ok;
}
