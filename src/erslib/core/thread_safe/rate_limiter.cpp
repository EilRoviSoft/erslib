#include "erslib/core/thread_safe/rate_limiter.hpp"

// std
#include <algorithm>
#include <cstddef>
#include <ranges>
#include <vector>

// ers
#include <erslib/core/exception.hpp>


// Member functions

ers::impl::RateLimiter::RateLimiter(size_t burst, duration interval, size_t capacity) :
    _interval(interval),
    _capacity(capacity),
    _low_water(capacity - std::min(capacity, std::max<size_t>(capacity / 8, 1))) {

    if (burst == 0)
        throw make_invalid_argument_error("RateLimiter: burst must be at least 1.");

    if (interval <= duration::zero())
        throw make_invalid_argument_error("RateLimiter: interval must be positive.");

    if (capacity == 0)
        throw make_invalid_argument_error("RateLimiter: capacity must be at least 1.");

    if (burst - 1 > static_cast<size_t>(duration::max() / interval))
        throw make_invalid_argument_error("RateLimiter: burst * interval overflows the clock.");

    _tolerance = interval * static_cast<duration::rep>(burst - 1);
}


// Accessors

ers::impl::RateLimiter::decision_t ers::impl::RateLimiter::try_acquire(std::string_view key, time_point now) {
    std::unique_lock lock(_mutex);

    auto it = _data.find(key);

    if (it == _data.end()) {
        if (!_make_room(now))
            return { .allowed = false, .remaining = 0, .retry_after = _next_sweep - now };

        it = _data.emplace(key, now).first;
    }


    auto arrival = std::max(it->second, now);
    auto backlog = arrival - now;

    if (backlog > _tolerance)
        return { .allowed = false, .remaining = 0, .retry_after = backlog - _tolerance };


    it->second = arrival + _interval;

    return {
        .allowed     = true,
        .remaining   = static_cast<size_t>((_tolerance - backlog) / _interval),
        .retry_after = duration::zero(),
    };
}

size_t ers::impl::RateLimiter::size() const {
    std::unique_lock lock(_mutex);
    return _data.size();
}


// Modifiers

void ers::impl::RateLimiter::reset(std::string_view key) {
    std::unique_lock lock(_mutex);
    _data.erase(key);
}

size_t ers::impl::RateLimiter::maintain(time_point now) {
    std::unique_lock lock(_mutex);
    return erase_if(_data, [&](const auto& it) { return it.second <= now; });
}


// Internal

bool ers::impl::RateLimiter::_is_blocked(time_point arrival, time_point now) const {
    return arrival - now > _tolerance;
}

bool ers::impl::RateLimiter::_make_room(time_point now) {
    if (_data.size() < _capacity)
        return true;

    if (now < _next_sweep)
        return false;


    erase_if(_data, [&](const auto& it) { return it.second <= now; });

    if (_data.size() > _low_water) {
        std::vector<time_point> evictable;
        evictable.reserve(_data.size());

        for (const auto& arrival : _data | std::views::values) {
            if (!_is_blocked(arrival, now))
                evictable.emplace_back(arrival);
        }

        if (!evictable.empty()) {
            auto count = std::min(_data.size() - _low_water, evictable.size());
            auto nth = evictable.begin() + static_cast<std::ptrdiff_t>(count - 1);

            std::ranges::nth_element(evictable, nth);

            erase_if(_data, [&](const auto& it) {
                return it.second <= *nth && !_is_blocked(it.second, now);
            });
        }
    }


    if (_data.size() < _capacity)
        return true;

    _next_sweep = now + _interval;
    return false;
}
