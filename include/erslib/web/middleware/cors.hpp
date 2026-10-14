#pragma once

// std
#include <chrono>
#include <string>
#include <vector>

// crow
#include <crow/http_request.h>
#include <crow/http_response.h>

// export
#include <erslib/export.hpp>


namespace ers::web::impl {
    struct cors_options_t {
        std::vector<std::string> origins;
        std::string allow_headers = "Authorization, Content-Type";
        std::string allow_methods = "GET, POST, OPTIONS";
        std::chrono::seconds max_age { 86400 };
    };


    class ERSLIB_EXPORT Cors {
    public:
        struct context {};


        // Accessors

        const cors_options_t& options() const;


        // Modifiers

        void configure(cors_options_t options);


        // Handlers

        void before_handle(crow::request& req, crow::response& res, context&);
        void after_handle(crow::request& req, crow::response& res, context&);


    private:
        cors_options_t _options;


        void _apply(const crow::request& req, crow::response& res) const;
    };
}
