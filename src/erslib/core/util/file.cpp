#include "erslib/core/util/file.hpp"

// std
#include <fstream>
#include <iterator>


std::string ERSLIB_EXPORT ers::impl::util::read_file(const std::filesystem::path& path) {
    auto&& stream = std::fstream(path, std::ios::in);
    return { std::istreambuf_iterator(stream), std::istreambuf_iterator<char>() };
}
