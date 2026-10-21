#include "erslib/core/type/version.hpp"

// std
#include <charconv>
#include <cstddef>
#include <system_error>


ers::Result<ers::impl::version_t> ers::impl::version_t::parse(std::string_view source) {
    const auto part = [](std::string_view what, size_t& out) {
        return std::from_chars(what.data(), what.data() + what.size(), out).ec == std::errc {};
    };


    const auto dot1 = source.find('.');
    if (dot1 == std::string_view::npos)
        return make_error("Can't convert string \"{}\" to type [T = version_t]", source);

    const auto dot2 = source.find('.', dot1 + 1);
    if (dot2 == std::string_view::npos)
        return make_error("Can't convert string \"{}\" to type [T = version_t]", source);


    version_t result {};

    if (!part(source.substr(0, dot1), result.major)
        || !part(source.substr(dot1 + 1, dot2 - dot1 - 1), result.minor)
        || !part(source.substr(dot2 + 1), result.patch)) {
        return make_error("Can't convert string \"{}\" to type [T = version_t]", source);
    }

    return result;
}
