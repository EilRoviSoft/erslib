#pragma once

// std
#include <string>
#include <vector>

// ers
#include <erslib/core/enum/util.hpp>


namespace ers::impl {
    constexpr std::vector<std::string_view> split_with_delim(std::string_view sv, char delim) {
        std::vector<std::string_view> result;
        size_t start = 0;

        while (start <= sv.size()) {
            size_t pos = sv.find(delim, start);

            if (pos == std::string_view::npos) {
                if (start < sv.size())
                    result.push_back(sv.substr(start));
                break;
            }

            if (pos > start)
                result.push_back(sv.substr(start, pos - start));
            start = pos + 1;
        }

        return result;
    }

    constexpr std::string combine_with_delim(const std::vector<std::string_view>& words, char delim) {
        std::string out;

        for (size_t i = 0; i < words.size(); ++i) {
            if (i)
                out += delim;
            out += to_lower(words[i]);
        }

        return out;
    }
}


namespace ers::impl {
    constexpr std::vector<std::string_view> split_with_case_diff(std::string_view sv) {
        std::vector<std::string_view> words;

        if (sv.empty())
            return words;

        size_t word_start = 0;
        for (size_t i = 1; i < sv.size(); ++i) {
            bool cur_upper = is_upper(sv[i]);
            bool prev_lower = is_lower(sv[i - 1]);
            bool prev_digit = is_digit(sv[i - 1]);

            if (cur_upper && (prev_lower || prev_digit)) {
                words.emplace_back(sv.substr(word_start, i - word_start));
                word_start = i;
            }
        }

        words.emplace_back(sv.substr(word_start));
        return words;
    }
}
