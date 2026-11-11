#pragma once


// Includes --------------------------------------------------------------------

#include <erslib/web/request.hpp>
#include <erslib/web/response.hpp>

#ifdef ERSLIB_WEB_HAS_LOGGING
#include <erslib/web/log.hpp>
#endif


// Exports ---------------------------------------------------------------------

namespace ers::web {
    using impl::web::failure_json;

    using impl::web::make_response;
    using impl::web::make_failure;
    using impl::web::make_empty;
    using impl::web::make_redirect;

    using impl::web::end_with_json;
    using impl::web::end_with_failure;

    using impl::web::read_body;
    using impl::web::read_header;
    using impl::web::read_cookie;
    using impl::web::bearer_token;
    using impl::web::query_number;
    using impl::web::query_flag;

#ifdef ERSLIB_WEB_HAS_LOGGING
    using impl::web::LogHandler;
#endif
}
