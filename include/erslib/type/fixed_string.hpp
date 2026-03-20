#pragma once

// ers
#include <erslib/convert/string.hpp>
#include <erslib/hashing/base.hpp>


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
struct ers::convert::to_string_backend<ers::fixed_string<N>> {
    constexpr std::string_view constexpr_value(const fixed_string<N>& what) const noexcept {
        return what.to_sv();
    }
};


template<size_t N, typename Policy>
struct ers::THashBase<ers::fixed_string<N>, Policy> {
    using type = const fixed_string<N>&;

    constexpr size_t operator()(type what, size_t seed = 0) const noexcept {
        return THashBase<const char[N], Policy> {}(what.value, seed);
    }
};
