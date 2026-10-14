#pragma once


// Includes

#include <erslib/web/request.hpp>
#include <erslib/web/response.hpp>

#include <erslib/web/middleware/body_limit.hpp>
#include <erslib/web/middleware/cors.hpp>
#include <erslib/web/middleware/json_body.hpp>
#include <erslib/web/middleware/rate_limit.hpp>

#ifdef ERSLIB_WEB_HAS_LOGGING
#include <erslib/web/log.hpp>
#endif


// Exports

namespace ers::web {
    using impl::failure_json;
    using impl::make_response;
    using impl::make_failure;
    using impl::make_ok;
    using impl::end_with_json;
    using impl::end_with_failure;

    using impl::read_body;
    using impl::read_header;
    using impl::bearer_token;
    using impl::query_number;
    using impl::query_flag;

    using impl::BodyLimit;

    using impl::cors_options_t;
    using impl::Cors;

    using impl::JsonBody;

    using impl::RateLimitPolicy;
    using impl::KeyedRateLimit;
    using impl::refuse_too_many;

#ifdef ERSLIB_WEB_HAS_LOGGING
    using impl::LogHandler;
#endif
}
