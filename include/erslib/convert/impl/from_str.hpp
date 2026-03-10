#pragma once

// std
#include <charconv>
#include <string>

// ers
#include <erslib/type/result.hpp>


// From string definitions

namespace ers {
    template<typename T>
    struct from_str {
        Result<T> operator()(std::string_view source) const = delete("default specialization");
    };
}


// Implementation for std conversions

namespace ers {
    template<typename T>
    concept HasStdFromChars = requires(const char* first, const char* last, T& value) {
        { std::from_chars(first, last, value) } -> std::convertible_to<std::from_chars_result>;
    };

    template<HasStdFromChars T>
    struct from_str<T> {
        template<typename... Args>
        Result<T> operator()(std::string_view source, Args&&... args) const {
            T result;

            if (!std::from_chars(source.begin(), source.end(), result, std::forward<Args>(args)...)) {
                return Unexpected<Error>(
                    Severity::Error,
                    "conversion_error",
                    std::format("Can't convert string \"{}\" to integral type", source)
                );
            }

            return result;
        }
    };
}
