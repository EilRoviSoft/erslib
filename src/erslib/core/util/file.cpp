#include "erslib/core/util/file.hpp"

// std
#include <fstream>


std::string ERSLIB_EXPORT ers::impl::util::read_file(const fs::path& path) {
    auto&& stream = std::fstream(path, std::ios::in);
    return { std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>() };
}
