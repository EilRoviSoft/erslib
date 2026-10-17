#pragma once

// std
#include <concepts>
#include <cstddef>
#include <string>

// crow
#include <crow/http_request.h>
#include <crow/http_response.h>
#include <crow/middleware.h>

// ers
#include <erslib/core/thread_safe/rate_limiter.hpp>
#include <erslib/core/type/optional.hpp>
#include <erslib/web/response.hpp>

// export
#include <erslib/export.hpp>


namespace ers::web::impl {
    template<typename T>
    concept RateLimitPolicy = requires {
        { T::reset_on_success } -> std::convertible_to<bool>;
    };


    void ERSLIB_EXPORT refuse_too_many(crow::response& res, thread_safe::RateLimiter::duration wait);


    template<RateLimitPolicy Policy>
    class KeyedRateLimit : public crow::ILocalMiddleware {
    public:
        struct context {
            std::string key;
        };


        // Accessors

        bool configured() const {
            return _limiter.has_value();
        }


        // Modifiers

        void configure(size_t burst, thread_safe::RateLimiter::duration interval, size_t capacity) {
            _limiter.emplace(burst, interval, capacity);
        }


        // Handlers

        template<typename AllContext>
        void before_handle(crow::request& req, crow::response& res, context& ctx, AllContext& all) {
            if (!_limiter)
                return end_with_failure(res, crow::INTERNAL_SERVER_ERROR, "Rate limiter is not configured.");

            ctx.key = Policy::key(req, all);

            if (ctx.key.empty())
                return;

            if (auto decision = _limiter->try_acquire(ctx.key); !decision)
                refuse_too_many(res, decision.retry_after);
        }

        void after_handle(crow::request&, crow::response& res, context& ctx) {
            if constexpr (Policy::reset_on_success) {
                if (_limiter && res.code == crow::OK && !ctx.key.empty())
                    _limiter->reset(ctx.key);
            }
        }


    private:
        optional<thread_safe::RateLimiter> _limiter;
    };
}


// Exports ---------------------------------------------------------------------

namespace ers::web {
    using impl::RateLimitPolicy;
    using impl::KeyedRateLimit;
    using impl::refuse_too_many;
}
