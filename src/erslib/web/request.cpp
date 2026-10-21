#include "erslib/web/request.hpp"

// ers
#include <erslib/core/stdlib/ctype.hpp>
#include <erslib/core/util/string.hpp>


// Details ---------------------------------------------------------------------

namespace {
    constexpr std::string_view bearer_prefix = "Bearer ";
}


// Implementation --------------------------------------------------------------

// Body

ers::Result<utl::Json> ers::web::impl::read_body(const crow::request& req) {
    return utl::try_from_string(req.body);
}


// Headers

ers::maybe<std::string> ers::web::impl::read_header(const crow::request& req, std::string_view name, size_t max_length) {
    auto value = util::keep_if(req.get_header_value(std::string(name)), is_print, max_length);

    if (value.empty())
        return none;

    return value;
}

std::string ers::web::impl::bearer_token(const crow::request& req) {
    const auto& header = req.get_header_value("Authorization");

    if (!std::string_view(header).starts_with(bearer_prefix))
        return {};

    return header.substr(bearer_prefix.size());
}

ers::maybe<std::string> ers::web::impl::read_cookie(const crow::request& req, std::string_view name) {
    auto [first, last] = req.headers.equal_range("Cookie");

    for (auto it = first; it != last; ++it) {
        std::string_view rest = it->second;

        while (!rest.empty()) {
            auto end = rest.find(';');
            auto pair = util::trim(rest.substr(0, end));
            rest = end == std::string_view::npos ? std::string_view() : rest.substr(end + 1);

            auto equals = pair.find('=');

            if (equals != std::string_view::npos && pair.substr(0, equals) == name)
                return std::string(pair.substr(equals + 1));
        }
    }

    return none;
}


// Query string

bool ers::web::impl::query_flag(const crow::query_string& qs, const std::string& key) {
    const char* raw = qs.get(key);
    return raw && std::string_view(raw) == "true";
}
