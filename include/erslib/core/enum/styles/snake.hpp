#pragma once

// ers
#include <erslib/core/meta.hpp>
#include <erslib/core/enum/util.hpp>
#include <erslib/core/enum/styles/base.hpp>
#include <erslib/core/enum/styles/common.hpp>


namespace ers::impl {
    struct snake_case_t;
}

template<>
struct ers::impl::case_style_traits<ers::impl::snake_case_t> {
    static constexpr std::vector<std::string_view> split(std::string_view sv) {
        return split_with_delim(sv, '_');
    }

    static constexpr std::string combine(const std::vector<std::string_view>& v) {
        return combine_with_delim(v, '_');
    }
};
