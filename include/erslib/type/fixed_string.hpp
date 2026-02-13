#pragma once


namespace ers {
    template<size_t N>
    struct fixed_string {
        char value[N];

        constexpr fixed_string(const char (&str)[N]) {
            for (size_t i = 0; i < N; i++)
                value[i] = str[i];
        }

        constexpr size_t size() const { return N - 1; }
    };

    template <size_t N>
    fixed_string(const char(&)[N]) -> fixed_string<N>;
}
