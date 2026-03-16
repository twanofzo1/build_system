#include "arguments.hpp"
#include <cassert>

Arguments::Arguments(int argc, char** argv) {
    for (int i = 0; i < argc; ++i) {
        this->args.push_back(argv[i]);
    }
}

int Arguments::size() const {
    return args.size();
}

std::string Arguments::operator[](u32 index) const {
    assert(index < args.size() && "Index out of bounds");
    return args[index];
}
