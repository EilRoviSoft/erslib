#include "erslib/core/util/string.hpp"


std::string ers::util::replace(std::string_view where, std::string_view from, std::string_view to, size_t estimated_replacements) {
    std::string result;


    if (where.empty())
        return result;


    const size_t estimated_buffer_size = static_cast<ptrdiff_t>(where.size())
        + (static_cast<ptrdiff_t>(to.size()) - static_cast<ptrdiff_t>(from.size()))
        * estimated_replacements;

    result.reserve(estimated_buffer_size);


    size_t prev = 0, curr;

    while ((curr = where.find(from, prev)) != std::string_view::npos) {
        result += where.substr(prev, curr - prev);
        result += to;
        prev = curr + from.size();
    }


    result += where.substr(prev);

    return result;
}
