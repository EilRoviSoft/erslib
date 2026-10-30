#include "support/temp_tree.hpp"

// std
#include <chrono>
#include <format>
#include <fstream>
#include <string>
#include <string_view>
#include <system_error>

// ers
#include <erslib/core/random.hpp>


// Definitions

namespace {
    constexpr int max_attempts = 16;
}


// Member functions

test::TempTree::TempTree(std::string_view prefix) :
    _root(make_root(prefix)) {
}

test::TempTree::~TempTree() {
    std::error_code ec;
    fs::remove_all(_root, ec);
}


// Accessors

const fs::path& test::TempTree::root() const noexcept {
    return _root;
}

fs::path test::TempTree::path(const fs::path& relative) const {
    return _root / relative;
}

std::string test::TempTree::read(const fs::path& relative) const {
    const auto target = path(relative);

    // 'read_file' gives an empty string for a file it can't open
    if (!fs::is_regular_file(target)) {
        throw ers::make_path_error("TempTree: '{}' is not a file",
            relative.generic_string());
    }

    return ers::util::read_file(target);
}


// Modifiers

void test::TempTree::write(const fs::path& relative, std::string_view text) const {
    const auto target = path(relative);
    fs::create_directories(target.parent_path());

    std::ofstream file(target, std::ios::binary);
    file << text;

    if (!file) {
        throw ers::make_path_error("TempTree: cannot write '{}'",
            relative.generic_string());
    }
}

void test::TempTree::touch(const fs::path& relative, std::chrono::seconds by) const {
    const auto target = path(relative);
    fs::last_write_time(target, fs::last_write_time(target) + by);
}

void test::TempTree::remove(const fs::path& relative) const {
    fs::remove_all(path(relative));
}


// Utility

fs::path test::TempTree::make_root(std::string_view prefix) {
    const auto base = fs::temp_directory_path();

    for (int attempt = 0; attempt < max_attempts; ++attempt) {
        auto candidate = base / std::format("{}_{:016x}", prefix, ers::random::entropy());

        if (fs::create_directory(candidate))
            return candidate;
    }

    throw ers::make_path_error("TempTree: cannot create a unique folder under '{}'",
        base.generic_string());
}
