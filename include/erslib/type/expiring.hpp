#pragma once

// std
#include <chrono>

// expiring_t

namespace ers {
    template<typename T, typename Clock = std::chrono::system_clock>
    struct expiring_t {
        T value;
        Clock::time_point updated_at;
        Clock::duration lifetime;

        expiring_t(T value, Clock::time_point created_at, Clock::duration lifetime) :
            value(std::move(value)),
            updated_at(created_at),
            lifetime(lifetime) {
        }

        void update(T v, Clock::time_point update_time = Clock::now()) {
            value = std::move(v);
            updated_at = update_time;
        }

        bool is_expired(Clock::time_point time = Clock::now()) const {
            return time >= updated_at + lifetime;
        }
    };
}
