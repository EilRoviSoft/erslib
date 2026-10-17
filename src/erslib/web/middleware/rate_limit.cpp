#include "erslib/web/middleware/rate_limit.hpp"

// std
#include <chrono>


// Implementation --------------------------------------------------------------

void ers::web::impl::refuse_too_many(crow::response& res, thread_safe::RateLimiter::duration wait) {
    auto seconds = std::chrono::ceil<std::chrono::seconds>(wait).count();

    auto body = failure_json("Too many attempts.");
    body["retry_after"] = seconds;

    res.set_header("Retry-After", std::to_string(seconds));
    end_with_json(res, crow::TOO_MANY_REQUESTS, body);
}
