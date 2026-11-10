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
#include <erslib/core/stdlib/compare.hpp>

// export
#include <erslib/export.hpp>


// Implementation --------------------------------------------------------------

namespace ers::web::impl {
    ERSLIB_EXPORT utl::Json failure_json(std::string_view message);


    ERSLIB_EXPORT crow::response make_response(crow::status code, const utl::Json& body);
    ERSLIB_EXPORT crow::response make_failure(crow::status code, std::string_view message);

    ERSLIB_EXPORT crow::response make_empty(crow::status code);
    ERSLIB_EXPORT crow::response make_redirect(crow::status code, std::string_view location);


    ERSLIB_EXPORT void end_with_json(crow::response& res, crow::status code, const utl::Json& body);
    ERSLIB_EXPORT void end_with_failure(crow::response& res, crow::status code, std::string_view message);
}


// Specialization --------------------------------------------------------------

namespace ers::web::impl {
    template<crow::status Code>
    struct Response {
        static constexpr crow::status code = Code;

        static constexpr bool is_success = is_between_from(std::to_underlying(Code), 200, 300);
        static constexpr bool is_redirect = is_between_from(std::to_underlying(Code), 300, 400);
        static constexpr bool is_failure = is_between_from(std::to_underlying(Code), 400, 600);

        static constexpr bool has_no_body = is_one_of<Code, crow::NO_CONTENT, crow::RESET_CONTENT, crow::NOT_MODIFIED>;


        // 2xx with a body

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


        // 204, 205, 304

        crow::response operator()() const
            requires (has_no_body) {
            return make_empty(Code);
        }


        // 3xx

        crow::response operator()(std::string_view location) const
            requires (is_redirect && !has_no_body) {
            return make_redirect(Code, location);
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
}


// Defines ---------------------------------------------------------------------

#define ERS_WEB_STATUSES(X) \
    X(ok, OK) \
    X(created, CREATED) \
    X(accepted, ACCEPTED) \
    X(non_authoritative_information, NON_AUTHORITATIVE_INFORMATION) \
    X(no_content, NO_CONTENT) \
    X(reset_content, RESET_CONTENT) \
    X(partial_content, PARTIAL_CONTENT) \
    X(multi_status, WEBDAV_MULTI_STATUS) \
    X(moved_permanently, MOVED_PERMANENTLY) \
    X(found, FOUND) \
    X(see_other, SEE_OTHER) \
    X(not_modified, NOT_MODIFIED) \
    X(temporary_redirect, TEMPORARY_REDIRECT) \
    X(permanent_redirect, PERMANENT_REDIRECT) \
    X(bad_request, BAD_REQUEST) \
    X(unauthorized, UNAUTHORIZED) \
    X(forbidden, FORBIDDEN) \
    X(not_found, NOT_FOUND) \
    X(method_not_allowed, METHOD_NOT_ALLOWED) \
    X(not_acceptable, NOT_ACCEPTABLE) \
    X(proxy_authentication_required, PROXY_AUTHENTICATION_REQUIRED) \
    X(conflict, CONFLICT) \
    X(gone, GONE) \
    X(precondition_failed, WEBDAV_PRECONDITION_FAILED) \
    X(payload_too_large, PAYLOAD_TOO_LARGE) \
    X(request_uri_too_long, WEBDAV_REQUEST_URI_TOO_LONG) \
    X(unsupported_media_type, UNSUPPORTED_MEDIA_TYPE) \
    X(range_not_satisfiable, RANGE_NOT_SATISFIABLE) \
    X(expectation_failed, EXPECTATION_FAILED) \
    X(unprocessable_entity, WEBDAV_UNPROCESSABLE_ENTITY) \
    X(locked, WEBDAV_LOCKED) \
    X(failed_dependency, WEBDAV_FAILED_DEPENDENCY) \
    X(precondition_required, PRECONDITION_REQUIRED) \
    X(too_many_requests, TOO_MANY_REQUESTS) \
    X(unavailable_for_legal_reasons, UNAVAILABLE_FOR_LEGAL_REASONS) \
    X(internal_server_error, INTERNAL_SERVER_ERROR) \
    X(not_implemented, NOT_IMPLEMENTED) \
    X(bad_gateway, BAD_GATEWAY) \
    X(service_unavailable, SERVICE_UNAVAILABLE) \
    X(gateway_timeout, GATEWAY_TIMEOUT) \
    X(variant_also_negotiates, VARIANT_ALSO_NEGOTIATES) \
    X(insufficient_storage, WEBDAV_INSUFFICIENT_STORAGE)

#define ERS_WEB_DEFINE_STATUS(NAME, CODE) \
    inline constexpr impl::Response<crow::CODE> make_##NAME {};

namespace ers::web {
    ERS_WEB_STATUSES(ERS_WEB_DEFINE_STATUS)
}

#undef ERS_WEB_DEFINE_STATUS
#undef ERS_WEB_STATUSES
