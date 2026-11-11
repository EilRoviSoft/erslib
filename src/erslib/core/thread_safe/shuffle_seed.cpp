#include "erslib/core/thread_safe/shuffle_seed.hpp"

// std
#include <mutex>
#include <ranges>
#include <shared_mutex>


// Implementation --------------------------------------------------------------

// Member functions

ers::impl::thread_safe::ShuffleSeeds::ShuffleSeeds(duration_t lifetime, u64 seed) :
    _lifetime(lifetime),
    _rng(seed) {
}


// Accessors

i64 ers::impl::thread_safe::ShuffleSeeds::acquire(std::string_view session, timestamp_t now) {
    boost::upgrade_lock read_lock(_mutex);

    auto it = _data.find(session);
    if (it != _data.end() && !it->second.is_expired(now))
        return it->second.value;


    boost::upgrade_to_unique_lock write_lock(read_lock);

    if (it == _data.end()) {
        return _data.try_emplace(session, expiring_t(_mint(), now, _lifetime))
            .first->second.value;
    }

    it->second.update(_mint(), now);
    return it->second.value;
}

size_t ers::impl::thread_safe::ShuffleSeeds::size() const {
    std::shared_lock lock(_mutex);
    return _data.size();
}


// Modifiers

i64 ers::impl::thread_safe::ShuffleSeeds::rotate(std::string_view session, timestamp_t now) {
    std::unique_lock lock(_mutex);

    auto it = _data.find(session);
    if (it == _data.end()) {
        return _data.try_emplace(session, expiring_t(_mint(), now, _lifetime))
            .first->second.value;
    }

    it->second.update(_mint(), now);
    return it->second.value;
}

void ers::impl::thread_safe::ShuffleSeeds::forget(std::string_view session) {
    std::unique_lock lock(_mutex);
    _data.erase(session);
}

size_t ers::impl::thread_safe::ShuffleSeeds::maintain(timestamp_t now) {
    std::unique_lock lock(_mutex);
    return erase_if(_data, [&](const auto& it) { return it.second.is_expired(now); });
}


// Generators

i64 ers::impl::thread_safe::ShuffleSeeds::_mint() {
    return static_cast<i64>(_rng());
}
