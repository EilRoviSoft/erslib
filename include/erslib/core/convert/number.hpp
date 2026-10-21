#pragma once

// std
#include <charconv>
#include <concepts>
#include <expected>

// ers
#include <erslib/core/meta.hpp>
#include <erslib/core/type/maybe.hpp>
#include <erslib/core/type/result.hpp>


namespace ers::impl {
    template<typename T>
    concept HasFromChars = requires(const char* first, const char* last, T& v) {
        { std::from_chars(first, last, v) } -> std::same_as<std::from_chars_result>;
    };
}


namespace ers::impl {
    template<HasFromChars T>
    T number_from_string(std::string_view text, int base = 10);

    template<HasFromChars T>
    Result<T> try_number_from_string(std::string_view text, int base = 10);

    template<HasFromChars T>
    maybe<T> maybe_number_from_string(std::string_view text, int base = 10);
}


// Exports ---------------------------------------------------------------------

namespace ers {
    using impl::number_from_string;
    using impl::try_number_from_string;
    using impl::maybe_number_from_string;
}


// Implementation --------------------------------------------------------------

#include "number.inl"
