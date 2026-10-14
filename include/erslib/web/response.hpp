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
    utl::Json ERSLIB_EXPORT failure_json(std::string_view message);

    crow::response ERSLIB_EXPORT make_response(crow::status code, const utl::Json& body);
    crow::response ERSLIB_EXPORT make_failure(crow::status code, std::string_view message);
    crow::response ERSLIB_EXPORT make_ok();

    void ERSLIB_EXPORT end_with_json(crow::response& res, crow::status code, const utl::Json& body);
    void ERSLIB_EXPORT end_with_failure(crow::response& res, crow::status code, std::string_view message);
}
