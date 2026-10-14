#pragma once

// std
#include <chrono>
#include <mutex>
#include <string_view>

// ers
#include <erslib/core/fwd.hpp>
#include <erslib/core/type/general.hpp>


namespace ers::impl {
    class RateLimiter {
    public:
        // Member types

        using clock_type = std::chrono::steady_clock;
        using time_point = clock_type::time_point;
        using duration = clock_type::duration;


        struct decision_t {
            bool allowed;
            size_t remaining;
            duration retry_after;


            explicit operator bool() const noexcept { return allowed; }
        };


        // Member functions

        RateLimiter(size_t burst, duration interval, size_t capacity);


        // Accessors

        decision_t try_acquire(std::string_view key, time_point now = clock_type::now());

        size_t size() const;


        // Modifiers

        void reset(std::string_view key);

        size_t maintain(time_point now = clock_type::now());


    private:
        mutable std::mutex _mutex;
        StringMap<time_point> _data;

        duration _interval;
        duration _tolerance;
        size_t _capacity;
        size_t _low_water;
        time_point _next_sweep {};


        bool _is_blocked(time_point arrival, time_point now) const;
        bool _make_room(time_point now);
    };
}


// Exports

namespace ers::thread_safe {
    using impl::RateLimiter;
}
