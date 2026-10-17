#include "erslib/web/middleware/json_body.hpp"

// std
#include <utility>

// ers
#include <erslib/web/request.hpp>
#include <erslib/web/response.hpp>


// Implementation --------------------------------------------------------------

// Handlers

void ers::web::impl::JsonBody::before_handle(crow::request& req, crow::response& res, context& ctx) {
    auto body = read_body(req);

    if (!body)
        return end_with_failure(res, crow::BAD_REQUEST, "Malformed JSON body.");

    ctx.body = std::move(*body);
}

void ers::web::impl::JsonBody::after_handle(crow::request&, crow::response&, context&) {}
