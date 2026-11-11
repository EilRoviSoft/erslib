#include "erslib/flux/search/pattern.hpp"


std::string flux::impl::search::like_pattern(std::string_view text, ELike kind) {
    std::string out;
    out.reserve(text.size() + 2);


    if (kind == ELike::Contains || kind == ELike::Suffix)
        out += '%';

    for (char c : text) {
        if (c == '\\' || c == '%' || c == '_')
            out += '\\';
        out += c;
    }

    if (kind == ELike::Contains || kind == ELike::Prefix)
        out += '%';


    return out;
}

std::string flux::impl::search::tsquery_word(std::string_view word, bool prefix) {
    std::string out = "'";

    for (char c : word) {
        if (c == '\'')
            out += '\'';
        else if (c == '\\')
            out += '\\';

        out += c;
    }

    out += '\'';

    if (prefix)
        out += ":*";

    return out;
}
