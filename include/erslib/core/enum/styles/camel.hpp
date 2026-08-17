#pragma once

// ers
#include <erslib/core/meta.hpp>
#include <erslib/core/enum/util.hpp>
#include <erslib/core/enum/styles/base.hpp>
#include <erslib/core/enum/styles/common.hpp>


namespace ers::impl {
    struct camel_case_t;
}

template<>
struct ers::impl::case_style_traits<ers::impl::camel_case_t> {
    static constexpr std::vector<std::string_view> split(std::string_view sv) {
        return split_with_case_diff(sv);
    }

    static constexpr std::string combine(const std::vector<std::string_view>& v) {
        std::string out;

        for (size_t i = 0; i < v.size(); ++i) {
            std::string word = to_lower(v[i]);
            out += i == 0 ? word : capitalise_first(word);
        }

        return out;
    }
};
