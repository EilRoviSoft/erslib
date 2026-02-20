#pragma once

// ers
#include <erslib/convert/string.hpp>


// Definition

namespace ers {
    template<size_t N>
    struct fixed_string {
        char value[N];

        constexpr fixed_string() {
            for (size_t i = 0; i < N; i++)
                value[i] = 0;
        }

        constexpr fixed_string(const char (&str)[N]) {
            for (size_t i = 0; i < N; i++)
                value[i] = str[i];
        }

        constexpr size_t size() const { return N - 1; }

        constexpr std::string_view to_sv() const noexcept {
            return { value, size() };
        }
    };

    template <size_t N>
    fixed_string(const char(&)[N]) -> fixed_string<N>;
}


// Utility

template<size_t N>
struct ers::to_sv<ers::fixed_string<N>> {
    constexpr std::string_view operator()(const fixed_string<N>& what) const noexcept {
        return what.to_sv();
    }
};
