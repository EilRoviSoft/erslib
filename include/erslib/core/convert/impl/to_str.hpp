#pragma once

// std
#include <charconv>
#include <stdexcept>
#include <string>


// to string backend

namespace ers::convert {
    template<typename T>
    struct to_string_backend;
}


// Traits

namespace ers::impl::convert {
    template<typename T>
    concept ToStringHasConstexprValue = requires(ers::convert::to_string_backend<T> backend, const T& value) {
        { backend.constexpr_value(value) } -> std::convertible_to<std::string_view>;
    };


    template<typename T>
    concept ToStringHasRuntimeValue = requires(ers::convert::to_string_backend<T> backend, const T& value) {
        { backend.runtime_value(value) } -> std::convertible_to<std::string>;
    };
}


// Utility functions

namespace ers::impl::convert {
    template<ToStringHasConstexprValue T>
    constexpr std::string_view to_sv(const T& value) noexcept {
        return ers::convert::to_string_backend<T> {}.constexpr_value(value);
    }

    template<typename T>
    std::string to_str(const T& value) {
        ers::convert::to_string_backend<T> backend;

        if constexpr (ToStringHasConstexprValue<T>) {
            return static_cast<std::string>(backend.constexpr_value(value));
        } else if constexpr (ToStringHasRuntimeValue<T>) {
            return backend.runtime_value(value);
        } else {
            throw std::runtime_error("Non-specialized implementation");
        }
    }
}


// Exports

namespace ers::convert {
    using impl::convert::to_sv;
    using impl::convert::to_str;
}


// Implementation for std conversions

namespace ers::convert {
    template<typename T>
    concept HasStdToChars = requires(const char* first, const char* last, T& value) {
        { std::from_chars(first, last, value) } -> std::convertible_to<std::from_chars_result>;
    };


    template<HasStdToChars T>
    struct to_string_backend<T> {
        std::string runtime_value(const T& value) const {
            std::array<char, std::numeric_limits<T>::digits10> buffer;

            if (!std::from_chars(buffer.data(), buffer.data() + buffer.size(), value))
                return "";

            return { buffer.begin(), buffer.end() };
        }
    };
}


// Concept

namespace ers::impl {
    template<typename T>
    concept ToStringConvertible = convert::ToStringHasConstexprValue<T> || convert::ToStringHasRuntimeValue<T>;
}
