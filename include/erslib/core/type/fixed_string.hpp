#pragma once

// ers
#include <erslib/core/hashing/base.hpp>


// Definition

namespace ers::impl {
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


        constexpr size_t size() const { return N; }


        constexpr size_t strlen() const noexcept {
            size_t result = 0;
            while (result < N && value[result] != '\0')
                result++;
            return result;
        }

        constexpr std::string_view to_sv() const noexcept {
            return { value, strlen() };
        }

        constexpr std::string_view view() const noexcept {
            return { value, strlen() };
        }
    };

    template <size_t N>
    fixed_string(const char(&)[N]) -> fixed_string<N>;
}


// Exports

namespace ers {
    using impl::fixed_string;
}



template<size_t N, typename Policy>
struct ers::THashBase<ers::fixed_string<N>, Policy> {
    using type = const fixed_string<N>&;

    constexpr size_t operator()(type what, size_t seed = 0) const noexcept {
        return THashBase<const char[N], Policy> {}(what.value, seed);
    }
};
