#pragma once

// std
#include <string_view>

// crow
#include <crow/common.h>
#include <crow/http_response.h>

// contrib
#include <erslib/contrib/json.hpp>

// export
#include <erslib/export.hpp>


namespace ers::web::impl {
    ERSLIB_EXPORT utl::Json failure_json(std::string_view message);

    ERSLIB_EXPORT crow::response make_response(crow::status code, const utl::Json& body);
    ERSLIB_EXPORT crow::response make_failure(crow::status code, std::string_view message);
    ERSLIB_EXPORT crow::response make_ok();

    ERSLIB_EXPORT void end_with_json(crow::response& res, crow::status code, const utl::Json& body);
    ERSLIB_EXPORT void end_with_failure(crow::response& res, crow::status code, std::string_view message);
}
