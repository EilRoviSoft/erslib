#pragma once

// crow
#include <crow/http_request.h>
#include <crow/http_response.h>
#include <crow/middleware.h>

// ers
#include <erslib/core/type/optional.hpp>

// contrib
#include <erslib/contrib/json.hpp>

// export
#include <erslib/export.hpp>


namespace ers::web::impl {
    class ERSLIB_EXPORT JsonBody : public crow::ILocalMiddleware {
    public:
        struct context {
            optional<utl::Json> body;
        };


        // Handlers

        void before_handle(crow::request& req, crow::response& res, context& ctx);
        void after_handle(crow::request&, crow::response&, context&);
    };
}


// Exports

namespace ers::web {
    using impl::JsonBody;
}
