#include "erslib/web/middleware/body_limit.hpp"

// ers
#include <erslib/web/response.hpp>


// Accessors

size_t ers::web::impl::BodyLimit::limit() const {
    return _limit;
}


// Modifiers

void ers::web::impl::BodyLimit::set_limit(size_t bytes) {
    _limit = bytes;
}


// Handlers

void ers::web::impl::BodyLimit::before_handle(crow::request& req, crow::response& res, context&) {
    if (req.body.size() > _limit)
        end_with_failure(res, crow::PAYLOAD_TOO_LARGE, "Request body is too large.");
}

void ers::web::impl::BodyLimit::after_handle(crow::request&, crow::response&, context&) {}
