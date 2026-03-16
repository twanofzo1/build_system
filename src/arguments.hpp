#pragma once
#include <vector>
#include <string>
#include "modern_types.h"

class Arguments {
private:
    std::vector<std::string> args;
public:
    Arguments(int argc, char** argv);
    int size() const;
    std::string operator[](u32 index) const;
};