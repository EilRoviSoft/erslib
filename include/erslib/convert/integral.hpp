#pragma once

// std
#include <charconv>
#include <concepts>
#include <string_view>

// ers
#include <erslib/error/include.hpp>

namespace ers {
    template<std::integral T>
    Result<T> from_chars(std::string_view sv, size_t base = 10) {
        T result;
        auto [ptr, ec] = std::from_chars(sv.data(), sv.data() + sv.size(), result, base);

        if (ec != std::errc {}) {
            return Unexpected<Error>(
                Severity::Error,
                "conversion_error",
                std::format("Can't convert string \"{}\" to integral type", sv)
            );
        }

        return result;
    }
}
