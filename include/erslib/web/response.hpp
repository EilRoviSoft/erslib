#pragma once

// std
#include <format>
#include <string_view>

// crow
#include <crow/common.h>
#include <crow/http_response.h>

// ers
#include <erslib/contrib/json.hpp>
#include <erslib/core/algorithm/constexpr.hpp>
#include <erslib/core/enum/generate.hpp>
#include <erslib/core/stdlib/compare.hpp>
#include <erslib/core/stdlib/ctype.hpp>

// export
#include <erslib/export.hpp>


namespace ers::web::impl {
    ERSLIB_EXPORT utl::Json failure_json(std::string_view message);

    ERSLIB_EXPORT crow::response make_response(crow::status code, const utl::Json& body);
    ERSLIB_EXPORT crow::response make_failure(crow::status code, std::string_view message);

    ERSLIB_EXPORT void end_with_json(crow::response& res, crow::status code, const utl::Json& body);
    ERSLIB_EXPORT void end_with_failure(crow::response& res, crow::status code, std::string_view message);
}


namespace ers::web::impl {
    struct ResponsePolicy {
        using enum_type = crow::status;

        static consteval bool is_enabled(crow::status code, std::string_view) {
            return code >= 200 && code != crow::MULTIPLE_CHOICES;
        }

        static consteval std::string get_name(crow::status, std::string_view id) {
            constexpr std::string_view prefix = "WEBDAV_";

            if (id.starts_with(prefix))
                id.remove_prefix(prefix.size());

            return std::format("make_{}", to_lower(id));
        }


        template<crow::status Code>
        struct Entry {
            static constexpr bool is_success = is_between_upto(std::to_underlying(Code), 200, 300);
            static constexpr bool is_redirect = is_between_upto(std::to_underlying(Code), 300, 400);
            static constexpr bool is_failure = is_between_upto(std::to_underlying(Code), 400, 600);

            static constexpr bool has_no_body = is_one_of<Code, crow::NO_CONTENT, crow::RESET_CONTENT, crow::NOT_MODIFIED>;


            // 2xx

            crow::response operator()() const
                requires (is_success && !has_no_body) {
                utl::Json body;
                body["ok"] = true;
                return make_response(Code, body);
            }

            crow::response operator()(const utl::Json& body) const
                requires (is_success && !has_no_body) {
                return make_response(Code, body);
            }


            crow::response operator()() const
                requires (has_no_body) {
                return crow::response(Code);
            }


            // 3xx

            crow::response operator()(std::string_view location) const
                requires (is_redirect && !has_no_body) {
                crow::response res(Code);
                res.add_header("Location", std::string(location));
                return res;
            }


            // 4xx, 5xx

            crow::response operator()(std::string_view message) const
                requires (is_failure) {
                return make_failure(Code, message);
            }

            template<typename... Args> requires (is_failure && sizeof...(Args) >= 1)
            crow::response operator()(std::format_string<Args...> fmt, Args&&... args) const {
                return make_failure(Code, std::format(fmt, std::forward<Args>(args)...));
            }
        };


        template<crow::status Code>
        using entry = Entry<Code>;
    };


    inline constexpr auto response = ers::enum_table<ResponsePolicy>;
}
