#include "erslib/core/algorithm/args.hpp"


// std
#include <utility>

// ers
#include <erslib/core/stdlib/ctype.hpp>


ers::Result<std::vector<std::string>> ers::impl::split_args(std::string_view line) {
    std::vector<std::string> out;
    std::string token;

    bool started = false;
    char quote = '\0';

    for (size_t i = 0; i < line.size(); ++i) {
        char c = line[i];


        if (quote != '\0') {
            if (c == quote)
                quote = '\0';
            else
                token.push_back(c);

            continue;
        }


        if (c == '\\') {
            if (i + 1 == line.size())
                return make_error("Trailing backlash.");

            char next = line[i + 1];

            if (quote == '"' && next != '"' && next != '\\') {
                token.push_back(c);
                continue;
            }

            token.push_back(next);
            started = true;
            ++i;

            continue;
        }


        if (quote == '"') {
            if (c == '"')
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
            out.push_back(std::move(token));
            token.clear();
            started = false;
        }
    }

    if (quote != '\0')
        return make_error("Unterminated {} quote.", quote == '"' ? "double" : "single");

    if (started)
        out.push_back(std::move(token));

    return out;
}


std::string ers::impl::quote_arg(std::string_view arg) {
    auto safe = [](char c) {
        return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9')
            || std::string_view("_@%+=:,./-").find(c) != std::string_view::npos;
    };

    bool plain = !arg.empty();

    for (char c : arg) {
        if (!safe(c)) {
            plain = false;
            break;
        }
    }

    if (plain)
        return std::string(arg);

    std::string out = "'";

    for (char c : arg) {
        if (c == '\'')
            out += "'\\''";
        else
            out.push_back(c);
    }

    out.push_back('\'');
    return out;
}

std::string ers::impl::join_args(const std::span<std::string>& args) {
    std::string out;

    for (const auto& arg : args) {
        if (!out.empty())
            out.push_back(' ');

        out += quote_arg(arg);
    }

    return out;
}
