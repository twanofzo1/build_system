#pragma once
#include <string>
#include <vector>
#include <unordered_map>

struct Build_target
{
    std::string name;
    std::string output_directory;
    std::vector<std::string> files;
    std::vector<std::string> flags;
};

struct Cache_entry
{
    std::string source_file;
    long long last_modified;
    std::vector<std::string> dependencies;
    std::vector<long long> dep_modified;
};

class Builder
{
private:
    std::string config_path;
    std::string cache_path;
    std::string compiler;
    std::string version;
    std::vector<Build_target> targets;
    std::unordered_map<std::string, Cache_entry> cache;

    bool parse_config();
    void load_cache();
    void save_cache();

    std::vector<std::string> split(const std::string& str, char delimiter);
    std::vector<std::string> get_dependencies(const std::string& source, const Build_target& target);
    long long get_mtime(const std::string& path);
    bool needs_rebuild(const std::string& source, const std::vector<std::string>& deps);

    std::string get_object_path(const std::string& source, const Build_target& target);
    bool compile_file(const std::string& source, const std::string& object, const Build_target& target);
    bool link_target(const std::vector<std::string>& objects, const Build_target& target);
    bool build_target(const Build_target& target);

public:
    Builder(const std::string& config_path, const std::string& cache_path);
    bool build();
};
