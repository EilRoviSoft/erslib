#include "erslib/core/algorithm/split/args.hpp"

// std
#include <utility>

// ers
#include <erslib/core/stdlib/ctype.hpp>


ers::Result<std::vector<std::string>> ers::impl::algo::split_args(std::string_view line) {
    std::vector<std::string> out;
    std::string token;

    bool started = false;
    char quote = '\0';

    for (char c : line) {
        if (quote != '\0') {
            if (c == quote)
                quote = '\0';
            else
                token.push_back(c);

            continue;
        }

        if (c == '\'' || c == '"') {
            quote = c;
            started = true;
            continue;
        }

        if (!is_space(c)) {
            token.push_back(c);
            started = true;
            continue;
        }

        if (started) {
            out.emplace_back(std::move(token));
            token.clear();
            started = false;
        }
    }

    if (quote != '\0')
        return make_error("Unterminated {} quote.", quote == '"' ? "double" : "single");

    if (started)
        out.emplace_back(std::move(token));

    return out;
}
