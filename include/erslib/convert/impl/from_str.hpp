#pragma once

// std
#include <charconv>
#include <string>

// ers
#include <erslib/meta/type_info.hpp>
#include <erslib/type/optional.hpp>
#include <erslib/type/result.hpp>


// From string definitions

namespace ers::convert {
    template<typename T>
    struct from_string_backend;
}


// Traits

namespace ers::internal {
    template<typename T>
    concept FromStringHasConstexprValue = requires(convert::from_string_backend<T> backend, std::string_view source) {
        { backend.constexpr_value(source) } -> std::convertible_to<optional<T>>;
    };


    template<typename T>
    concept FromStringHasRuntimeValue = requires(convert::from_string_backend<T> backend, std::string_view source) {
        { backend.runtime_value(source) } -> std::convertible_to<Result<T>>;
    };
}


// Utility functions

namespace ers::convert {
    template<internal::FromStringHasConstexprValue T>
    [[nodiscard]]
    constexpr optional<T> from_str_constexpr(std::string_view source) noexcept {
        return from_string_backend<T> {}.constexpr_value(source);
    }

    template<typename T>
    [[nodiscard]]
    Result<T> from_str(std::string_view source) {
        from_string_backend<T> backend;

        if constexpr (internal::FromStringHasConstexprValue<T>) {
            auto r = backend.constexpr_value(source);
            if (!r) {
                return Unexpected<Error>(
                    Severity::Error,
                    "parse_error",
                    "Can't convert string \"{}\" to type [T = {}]",
                    source, meta::type_name_v<T>
                );
            }

            return *r;
        } else if constexpr (internal::FromStringHasRuntimeValue<T>) {
            return backend.runtime_value(source);
        } else {
            throw std::runtime_error("Non-specialized implementation");
        }
    }
}


// Implementation for std conversions

namespace ers::convert {
    template<typename T>
    concept HasStdFromChars = requires(const char* first, const char* last, T& value) {
        { std::from_chars(first, last, value) } -> std::convertible_to<std::from_chars_result>;
    };


    template<HasStdFromChars T>
    struct from_string_backend<T> {
        Result<T> runtime_value(std::string_view source) const {
            T result;

            if (std::from_chars(source.data(), source.data() + source.size(), result).ec != std::errc {}) {
                return Unexpected<Error>(
                    Severity::Error,
                    "conversion_error",
                    "Can't convert string \"{}\" to type [T = {}]",
                    source,
                    meta::type_name_v<T>
                );
            }

            return result;
        }
    };
}


// Concept

namespace ers::convert {
    template<typename T>
    concept FromStringConvertible = internal::FromStringHasConstexprValue<T> || internal::FromStringHasRuntimeValue<T>;
}
