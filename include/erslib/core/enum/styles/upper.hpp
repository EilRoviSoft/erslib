#pragma once

// ers
#include <erslib/core/meta.hpp>
#include <erslib/core/enum/util.hpp>
#include <erslib/core/enum/styles/base.hpp>


namespace ers::impl {
    struct upper_case_t;
}

template<>
struct ers::impl::case_style_traits<ers::impl::upper_case_t> {
    static constexpr std::vector<std::string_view> split(std::string_view identifier) {
        return { identifier };
    }

    static constexpr std::string combine(const std::vector<std::string_view>& words) {
        return to_upper(words.empty() ? std::string_view {} : words[0]);
    }
};
