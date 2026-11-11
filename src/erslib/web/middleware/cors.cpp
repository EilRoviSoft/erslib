#include "erslib/web/middleware/cors.hpp"

// std
#include <algorithm>
#include <utility>


// Implementation --------------------------------------------------------------

// Accessors

const ers::impl::web::cors_options_t& ers::impl::web::Cors::options() const {
    return _options;
}


// Modifiers

void ers::impl::web::Cors::configure(cors_options_t options) {
    _options = std::move(options);
}


// Handlers

void ers::impl::web::Cors::before_handle(crow::request& req, crow::response& res, context&) {
    _apply(req, res);
}

void ers::impl::web::Cors::after_handle(crow::request& req, crow::response& res, context&) {
    _apply(req, res);
}


void ers::impl::web::Cors::_apply(const crow::request& req, crow::response& res) const {
    res.set_header("Vary", "Origin");

    const auto& origin = req.get_header_value("Origin");

    if (origin.empty() || std::ranges::find(_options.origins, origin) == _options.origins.end()) {
        res.headers.erase("Access-Control-Allow-Origin");
        return;
    }

    res.set_header("Access-Control-Allow-Origin", origin);
    res.set_header("Access-Control-Allow-Headers", _options.allow_headers);
    res.set_header("Access-Control-Allow-Methods", _options.allow_methods);
    res.set_header("Access-Control-Max-Age", std::to_string(_options.max_age.count()));
}
