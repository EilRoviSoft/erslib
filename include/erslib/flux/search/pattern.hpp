#pragma once

// std
#include <ranges>
#include <string>
#include <string_view>

// ers
#include <erslib/core/type/general.hpp>

// export
#include <erslib/export.hpp>


namespace flux::impl::search {
    enum class ELike : u8 {
        Contains,
        Prefix,
        Suffix,
        Exact,
    };
}

namespace flux::impl::search {
    ERSLIB_EXPORT std::string like_pattern(std::string_view text, ELike kind = ELike::Contains);


    ERSLIB_EXPORT std::string tsquery_word(std::string_view word, bool prefix = true);

    template<std::ranges::input_range R>
        requires std::convertible_to<std::ranges::range_reference_t<R>, std::string_view>
    std::string tsquery_words(R&& words, std::string_view joiner = " & ", bool prefix = true) {
        std::string out;

        for (const auto& it : words) {
            std::string_view word = it;

            if (word.empty())
                continue;

            if (!out.empty())
                out += joiner;

            out += tsquery_word(word, prefix);
        }

        return out;
    }
}


// Exports----------------------------------------------------------------------

namespace flux::search {
    using impl::search::ELike;

    using impl::search::like_pattern;
    using impl::search::tsquery_word;
    using impl::search::tsquery_words;
}
