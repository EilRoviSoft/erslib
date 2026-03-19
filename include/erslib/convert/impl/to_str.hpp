#pragma once

// std
#include <charconv>
#include <string>


// ToStringBackend

namespace ers::convert {
    template<typename T>
    struct to_string_backend;
}


// Traits

namespace ers::internal {
    template<typename T>
    concept ToStringHasConstexprValue = requires(convert::to_string_backend<T> backend, const T& value) {
        { backend.constexpr_value(value) } -> std::convertible_to<std::string_view>;
    };


    template<typename T>
    concept ToStringHasRuntimeValue = requires(convert::to_string_backend<T> backend, const T& value) {
        { backend.runtime_value(value) } -> std::convertible_to<std::string>;
    };
}


// Utility functions

namespace ers::convert {
    template<internal::ToStringHasConstexprValue T>
    constexpr std::string_view to_sv(const T& value) noexcept {
        return to_string_backend<T> {}.constexpr_value(value);
    }

    template<typename T>
    std::string to_str(const T& value) {
        to_string_backend<T> backend;

        if constexpr (internal::ToStringHasConstexprValue<T>) {
            return static_cast<std::string>(backend.constexpr_value(value));
        } else if constexpr (internal::ToStringHasRuntimeValue<T>) {
            return backend.runtime_value(value);
        } else {
            throw std::runtime_error("Non-specialized implementation");
        }
    }
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
