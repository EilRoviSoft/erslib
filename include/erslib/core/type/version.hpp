#pragma once

// std
#include <charconv>
#include <cstddef>
#include <string_view>

// ers
#include <erslib/core/type/result.hpp>


// Definition

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


// Implementation

namespace ers::impl {
    inline Result<version_t> version_t::parse(std::string_view source) {
        const auto part = [](std::string_view what, size_t& out) {
            return std::from_chars(what.data(), what.data() + what.size(), out).ec == std::errc {};
        };


        const auto dot1 = source.find('.');
        if (dot1 == std::string_view::npos)
            return make_error("Can't convert string \"{}\" to type [T = version_t]", source);

        const auto dot2 = source.find('.', dot1 + 1);
        if (dot2 == std::string_view::npos)
            return make_error("Can't convert string \"{}\" to type [T = version_t]", source);


        version_t result {};

        if (!part(source.substr(0, dot1), result.major)
            || !part(source.substr(dot1 + 1, dot2 - dot1 - 1), result.minor)
            || !part(source.substr(dot2 + 1), result.patch)) {
            return make_error("Can't convert string \"{}\" to type [T = version_t]", source);
        }

        return result;
    }
}
