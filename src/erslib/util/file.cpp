#include "erslib/util/file.hpp"

// std
#include <fstream>


std::string ERSLIB_EXPORT ers::util::read_file(const fs::path& path) {
    auto&& stream = std::fstream(path, std::ios::in);
    return { std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>() };
}
