#pragma once

// std
#include <chrono>

// expiring_t

namespace ers {
    template<typename T, typename TClock = std::chrono::system_clock>
    struct expiring_t {
        T value;
        TClock::time_point updated_at;
        TClock::duration lifetime;

        expiring_t(T value, TClock::time_point created_at, TClock::duration lifetime) :
            value(std::move(value)),
            updated_at(created_at),
            lifetime(lifetime) {
        }

        void update(T v, TClock::time_point update_time = TClock::now()) {
            value = std::move(v);
            updated_at = update_time;
        }

        bool is_expired(TClock::time_point time = TClock::now()) const {
            return time >= updated_at + lifetime;
        }
    };
}
