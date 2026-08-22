#pragma once

// std
#include <chrono>
#include <random>

// boost
#include <boost/thread/shared_mutex.hpp>

// ers
#include <erslib/core/fwd.hpp>
#include <erslib/core/random.hpp>
#include <erslib/core/type/expiring.hpp>
#include <erslib/core/type/time.hpp>


namespace ers::impl {
    class ShuffleSeeds {
    public:
        // Member functions

        explicit ShuffleSeeds(duration_t lifetime, u64 seed = entropy());


        // Accessors

        i64 acquire(std::string_view session, timestamp_t now = clock_t::now());

        size_t size() const;


        // Modifiers

        i64 rotate(std::string_view session, timestamp_t now = clock_t::now());

        void forget(std::string_view session);

        size_t maintain(timestamp_t now = clock_t::now());


    private:
        mutable boost::shared_mutex _mutex;
        StringMap<expiring_t<i64>> _data;
        duration_t _lifetime;
        std::mt19937_64 _rng;


        i64 _mint();
    };
}


// Exports

namespace ers::thread_safe {
    using impl::ShuffleSeeds;
}
