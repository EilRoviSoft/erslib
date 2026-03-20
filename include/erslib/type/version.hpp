#pragma once

// ers
#include <erslib/convert/string.hpp>


// Definition

namespace ers {
    struct version_t {
        size_t major;
        size_t minor;
        size_t patch;


        constexpr bool operator==(const version_t& other) const noexcept = default;
    };
}


// Utility

template<>
struct ers::convert::from_string_backend<ers::version_t> {
    Result<version_t> runtime_value(std::string_view source) const {
        version_t result;


        auto dot1 = source.find('.');
        if (dot1 == std::string_view::npos) {
            return Unexpected<Error>(
                Severity::Error,
                "parse_error",
                "Can't convert string \"{}\" to type [T = version_t]",
                source
            );
        }

        auto dot2 = source.find('.', dot1 + 1);
        if (dot2 == std::string_view::npos) {
            return Unexpected<Error>(
                Severity::Error,
                "parse_error",
                "Can't convert string \"{}\" to type [T = version_t]",
                source
            );
        }


        if (auto r = from_str<size_t>(source.substr(0, dot1)); !r)
            return Unexpected(r.error());
        else
            result.major = *r;

        if (auto r = from_str<size_t>(source.substr(dot1 + 1, dot2 - dot1 - 1)); !r)
            return Unexpected(r.error());
        else
            result.minor = *r;

        if (auto r = from_str<size_t>(source.substr(dot2 + 1)); !r)
            return Unexpected(r.error());
        else
            result.patch = *r;


        return result;
    }
};


template<>
struct ers::convert::to_string_backend<ers::version_t> {
    std::string runtime_value(const version_t& value) const {
        return std::format("{}.{}.{}", value.major, value.minor, value.patch);
    }
};
