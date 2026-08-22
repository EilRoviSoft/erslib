#include "erslib/core/random/entropy.hpp"

// std
#include <random>
#include <mutex>


namespace {
    std::random_device& device() {
        static std::random_device instance;
        return instance;
    }

    std::mutex guard;
}


u64 ers::impl::entropy() {
    std::lock_guard lock(guard);
    
    auto& source = device();
    u64 seed0 = source();
    u32 seed1 = source();

    return (seed0 << 32) ^ seed1;
}
