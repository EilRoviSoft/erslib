#pragma once

// ers
#include <erslib/core/meta.hpp>
#include <erslib/core/enum/util.hpp>
#include <erslib/core/enum/styles/base.hpp>
#include <erslib/core/enum/styles/common.hpp>


namespace ers::impl {
    struct pascal_case_t;
}

template<>
struct ers::impl::case_style_traits<ers::impl::pascal_case_t> {
    static constexpr std::vector<std::string_view> split(std::string_view sv) {
        return split_with_case_diff(sv);
    }

    static constexpr std::string combine(const std::vector<std::string_view>& v) {
        std::string out;

        for (std::string_view word : v)
            out += capitalise_first(to_lower(word));

        return out;
    }
};
