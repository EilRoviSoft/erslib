#pragma once

// std
#include <algorithm>
#include <string>
#include <string_view>

// crow
#include <crow/http_request.h>
#include <crow/query_string.h>

// ers
#include <erslib/core/convert/number.hpp>
#include <erslib/core/type/maybe.hpp>
#include <erslib/core/type/result.hpp>

// contrib
#include <erslib/contrib/json.hpp>

// export
#include <erslib/export.hpp>


namespace ers::web::impl {
    ERSLIB_EXPORT Result<utl::Json> read_body(const crow::request& req);

    ERSLIB_EXPORT maybe<std::string> read_header(const crow::request& req, std::string_view name, size_t max_length = 256);
    ERSLIB_EXPORT std::string bearer_token(const crow::request& req);
    ERSLIB_EXPORT maybe<std::string> read_cookie(const crow::request& req, std::string_view name);


    template<typename T>
    T query_number(const crow::query_string& qs, const std::string& key, T fallback, T min, T max) {
        const char* raw = qs.get(key);

        if (!raw)
            return fallback;

        auto parsed = maybe_number_from_string<T>(raw);

        return parsed ? std::clamp(*parsed, min, max) : fallback;
    }

    ERSLIB_EXPORT bool query_flag(const crow::query_string& qs, const std::string& key);
}
