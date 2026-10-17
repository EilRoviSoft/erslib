#pragma once

// std
#include <string_view>

// ers
#include <erslib/core/type/result.hpp>


// Implementation --------------------------------------------------------------

namespace ers::impl {
    struct version_t {
        size_t major;
        size_t minor;
        size_t patch;


        constexpr auto operator<=>(const version_t& other) const noexcept = default;


        // Parses "<major>.<minor>.<patch>".
        static Result<version_t> parse(std::string_view source);
    };
}

namespace ers {
    using impl::version_t;
}
