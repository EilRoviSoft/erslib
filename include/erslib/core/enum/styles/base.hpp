#pragma once

// std
#include <string>
#include <vector>


namespace ers::impl {
    template<typename Style>
    struct case_style_traits {
        static constexpr std::vector<std::string_view> split(std::string_view sv) = delete("non-specialized template");
        static constexpr std::string combine(const std::vector<std::string_view>& v) = delete("non-specialized template");
    };

    struct base_case_t {};
}
