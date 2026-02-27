#pragma once

// std
#include <chrono>

// boost
#include <boost/thread/locks.hpp>
#include <boost/thread/shared_mutex.hpp>

// ers
#include <erslib/type/result.hpp>
#include <erslib/type/expiring.hpp>


namespace ers {
    template<typename T, typename Clock = std::chrono::system_clock>
    class ITimedObject {
    public:
        using value_type = expiring_t<T, Clock>;


        // Constructors

        explicit ITimedObject(Clock::duration lifetime) :
            m_expiring(T(), {}, lifetime) {
        }

        ITimedObject(Clock::duration lifetime, T value, Clock::time_point created_at = Clock::now()) :
            m_expiring(value, lifetime, created_at) {
        }


        ITimedObject(const ITimedObject& other) :
            m_expiring(other.m_expiring) {
        }
        ITimedObject& operator=(const ITimedObject& other) {
            m_expiring = other.m_expiring;
            return *this;
        }


        ITimedObject(ITimedObject&& other) noexcept :
            m_expiring(std::move(other.m_expiring)) {
        }
        ITimedObject& operator=(ITimedObject&& other) noexcept {
            m_expiring = std::move(other.m_expiring);
            return *this;
        }


        // Destructors

        virtual ~ITimedObject() = default;


        // Observers

        [[nodiscard]]
        Result<T> get() const {
            boost::upgrade_lock read_lock(_mutex);

            if (m_expiring.is_expired()) {
                boost::upgrade_to_unique_lock write_lock(read_lock);

                if (auto s = load(); !s) {
                    m_expiring.value = T();
                    return Unexpected(s.error());
                }
            }

            return m_expiring.value;
        }


    protected:
        mutable value_type m_expiring;

        virtual Status load() const = 0;


    private:
        mutable boost::upgrade_mutex _mutex;
    };
}
