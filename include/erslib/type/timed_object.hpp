#pragma once

// std
#include <chrono>

// boost
#include <boost/thread/locks.hpp>
#include <boost/thread/shared_mutex.hpp>

// ers
#include <erslib/error/include.hpp>
#include <erslib/type/expiring.hpp>

namespace ers {
    template<typename T, typename Clock = std::chrono::system_clock>
    class ITimedObject {
    public:
        explicit ITimedObject(Clock::duration duration) :
            m_expiring(T(), {}, duration) {
        }
        ITimedObject(Clock::duration duration, T value, Clock::time_point created_at = Clock::now()) :
            m_expiring(value, duration, created_at) {
        }

        virtual ~ITimedObject() = default;

        Result<T> get() const {
            boost::upgrade_lock read_lock(_mutex);

            if (m_expiring.is_expired()) {
                boost::upgrade_to_unique_lock write_lock(read_lock);

                if (auto s = this->load(); !s) {
                    m_expiring.value = T();
                    return Unexpected(s.error());
                }
            }

            return m_expiring.value;
        }

    protected:
        mutable expiring_t<T, Clock> m_expiring;

        virtual Status load() const = 0;

    private:
        mutable boost::upgrade_mutex _mutex;
    };
}
