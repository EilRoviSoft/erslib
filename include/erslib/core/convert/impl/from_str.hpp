#pragma once

// std
#include <charconv>
#include <string>

// ers
#include <erslib/core/meta.hpp>
#include <erslib/core/type/optional.hpp>
#include <erslib/core/type/result.hpp>


static_assert(requires { typename ers::meta::type_name<int>; }, "type_info.hpp not included");


// from string backend

namespace ers::convert {
    template<typename T>
    struct from_string_backend;
}


// Traits

namespace ers::impl::convert {
    template<typename T>
    concept FromStringHasConstexprValue = requires(ers::convert::from_string_backend<T> backend, std::string_view source) {
        { backend.constexpr_value(source) } -> std::convertible_to<optional<T>>;
    };


    template<typename T>
    concept FromStringHasRuntimeValue = requires(ers::convert::from_string_backend<T> backend, std::string_view source) {
        { backend.runtime_value(source) } -> std::convertible_to<Result<T>>;
    };
}


// Utility functions

namespace ers::impl::convert {
    template<FromStringHasConstexprValue T>
    [[nodiscard]]
    constexpr optional<T> from_str_constexpr(std::string_view source) noexcept {
        return ers::convert::from_string_backend<T> {}.constexpr_value(source);
    }

    template<typename T>
    [[nodiscard]]
    Result<T> from_str(std::string_view source) {
        ers::convert::from_string_backend<T> backend;

        if constexpr (FromStringHasConstexprValue<T>) {
            return backend.constexpr_value(source);
        } else if constexpr (FromStringHasRuntimeValue<T>) {
            auto r = backend.runtime_value(source);
            if (!r) {
                return make_error("Can't convert string \"{}\" to type [T = {}]",
                    source, meta::type_name_v<T>);
            }

            return *r;
        } else {
            throw std::runtime_error("Non-specialized implementation");
        }
    }
}


// Exports

namespace ers::convert {
    using impl::convert::from_str_constexpr;
    using impl::convert::from_str;
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
                return make_error("Can't convert string \"{}\" to type [T = {}]",
                    source, meta::type_name_v<T>);
            }

            return result;
        }
    };
}


// Concept

namespace ers::impl {
    template<typename T>
    concept FromStringConvertible = convert::FromStringHasConstexprValue<T> || convert::FromStringHasRuntimeValue<T>;
}
