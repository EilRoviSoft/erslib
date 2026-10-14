#pragma once

// std
#include <cstddef>

// crow
#include <crow/http_request.h>
#include <crow/http_response.h>

// export
#include <erslib/export.hpp>


namespace ers::web::impl {
    class ERSLIB_EXPORT BodyLimit {
    public:
        struct context {};


        // Member data

        static constexpr size_t default_limit = 1 << 20;


        // Accessors

        size_t limit() const;


        // Modifiers

        void set_limit(size_t bytes);


        // Handlers

        void before_handle(crow::request& req, crow::response& res, context&);
        void after_handle(crow::request&, crow::response&, context&);


    private:
        size_t _limit = default_limit;
    };
}
