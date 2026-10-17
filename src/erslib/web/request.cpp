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

ers::optional<std::string> ers::web::impl::read_header(const crow::request& req, std::string_view name, size_t max_length) {
    auto value = util::keep_if(req.get_header_value(std::string(name)), is_print, max_length);

    if (value.empty())
        return nullopt;

    return value;
}

std::string ers::web::impl::bearer_token(const crow::request& req) {
    const auto& header = req.get_header_value("Authorization");

    if (!std::string_view(header).starts_with(bearer_prefix))
        return {};

    return header.substr(bearer_prefix.size());
}


// Query string

bool ers::web::impl::query_flag(const crow::query_string& qs, const std::string& key) {
    const char* raw = qs.get(key);
    return raw && std::string_view(raw) == "true";
}
