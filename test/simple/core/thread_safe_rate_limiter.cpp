// catch2
#include <catch2/catch_test_macros.hpp>

// std
#include <atomic>
#include <chrono>
#include <stdexcept>
#include <string_view>
#include <thread>
#include <vector>

// ers
#include <erslib/core/thread_safe/rate_limiter.hpp>

// test
#include "support/test_case.hpp"


using namespace std::chrono_literals;


namespace {
    using Limiter = ers::thread_safe::RateLimiter;

    constexpr size_t burst = 3;
    constexpr auto interval = 10min;
    constexpr size_t capacity = 1000;

    Limiter::time_point t0() {
        return Limiter::time_point() + 24h;
    }

    void exhaust(Limiter& limiter, std::string_view key, Limiter::time_point at) {
        for (size_t i = 0; i < burst; i++)
            REQUIRE(limiter.try_acquire(key, at).allowed);
    }
}


ERS_TEST_CASE("burst", "[core]") {
    Limiter limiter(burst, interval, capacity);

    const auto first = limiter.try_acquire("key", t0());

    CHECK(first.allowed);
    CHECK(static_cast<bool>(first));
    CHECK(first.remaining == 2);
    CHECK(first.retry_after == 0s);

    CHECK(limiter.try_acquire("key", t0()).remaining == 1);
    CHECK(limiter.try_acquire("key", t0()).remaining == 0);

    const auto blocked = limiter.try_acquire("key", t0());

    CHECK_FALSE(blocked.allowed);
    CHECK_FALSE(static_cast<bool>(blocked));
    CHECK(blocked.remaining == 0);
    CHECK(blocked.retry_after == interval);
}

ERS_TEST_CASE("interval", "[core]") {
    Limiter limiter(burst, interval, capacity);

    exhaust(limiter, "key", t0());

    CHECK(limiter.try_acquire("key", t0() + 5min).retry_after == 5min);

    const auto refilled = limiter.try_acquire("key", t0() + interval);

    CHECK(refilled.allowed);
    CHECK(refilled.remaining == 0);
    CHECK(limiter.try_acquire("key", t0() + interval).retry_after == interval);
}

ERS_TEST_CASE("blocked", "[core]") {
    Limiter limiter(burst, interval, capacity);

    exhaust(limiter, "key", t0());

    for (size_t i = 0; i < 100; i++)
        CHECK_FALSE(limiter.try_acquire("key", t0()).allowed);

    CHECK(limiter.try_acquire("key", t0() + interval).allowed);
}

ERS_TEST_CASE("refill", "[core]") {
    Limiter limiter(burst, interval, capacity);

    exhaust(limiter, "key", t0());

    CHECK(limiter.try_acquire("key", t0() + interval * static_cast<int>(burst)).remaining == burst - 1);
}

ERS_TEST_CASE("keys", "[core]") {
    Limiter limiter(burst, interval, capacity);

    exhaust(limiter, "a", t0());

    CHECK_FALSE(limiter.try_acquire("a", t0()).allowed);
    CHECK(limiter.try_acquire("b", t0()).remaining == burst - 1);
}

ERS_TEST_CASE("reset", "[core]") {
    Limiter limiter(burst, interval, capacity);

    exhaust(limiter, "key", t0());
    limiter.reset("key");

    CHECK(limiter.size() == 0);
    CHECK(limiter.try_acquire("key", t0()).remaining == burst - 1);
}

ERS_TEST_CASE("maintain", "[core]") {
    Limiter limiter(burst, interval, capacity);

    limiter.try_acquire("old", t0());
    limiter.try_acquire("recent", t0() + 5min);

    CHECK(limiter.size() == 2);
    CHECK(limiter.maintain(t0() + interval) == 1);
    CHECK(limiter.size() == 1);
    CHECK(limiter.try_acquire("recent", t0() + interval).remaining == 1);
}

ERS_TEST_CASE("eviction", "[core]") {
    Limiter limiter(burst, interval, 4);

    exhaust(limiter, "victim", t0());
    REQUIRE_FALSE(limiter.try_acquire("victim", t0()).allowed);

    limiter.try_acquire("junk-1", t0() + 1s);
    limiter.try_acquire("junk-2", t0() + 2s);
    limiter.try_acquire("junk-3", t0() + 3s);
    REQUIRE(limiter.size() == 4);

    CHECK(limiter.try_acquire("fresh", t0() + 4s).allowed);
    CHECK(limiter.size() == 4);
    CHECK_FALSE(limiter.try_acquire("victim", t0() + 5s).allowed);
}

ERS_TEST_CASE("lockouts", "[core]") {
    Limiter limiter(1, interval, 2);

    CHECK(limiter.try_acquire("a", t0()).allowed);
    CHECK(limiter.try_acquire("b", t0()).allowed);

    const auto refused = limiter.try_acquire("c", t0() + 1s);

    CHECK_FALSE(refused.allowed);
    CHECK(refused.retry_after == interval);
    CHECK(limiter.size() == 2);
    CHECK_FALSE(limiter.try_acquire("a", t0() + 1s).allowed);

    CHECK(limiter.try_acquire("c", t0() + interval + 1s).allowed);
}

ERS_TEST_CASE("concurrent", "[core]") {
    Limiter limiter(burst, interval, capacity);

    constexpr size_t workers = 8;
    constexpr size_t rounds = 100;

    std::atomic<size_t> allowed = 0;
    std::vector<std::thread> threads;

    for (size_t w = 0; w < workers; w++) {
        threads.emplace_back([&] {
            for (size_t i = 0; i < rounds; i++) {
                if (limiter.try_acquire("hot", t0()).allowed)
                    allowed++;
            }
        });
    }

    for (auto& it : threads)
        it.join();

    CHECK(allowed.load() == burst);
    CHECK(limiter.size() == 1);
}

ERS_TEST_CASE("invalid config", "[core]") {
    CHECK_THROWS_AS(Limiter(0, interval, capacity), std::invalid_argument);
    CHECK_THROWS_AS(Limiter(burst, 0s, capacity), std::invalid_argument);
    CHECK_THROWS_AS(Limiter(burst, -interval, capacity), std::invalid_argument);
    CHECK_THROWS_AS(Limiter(burst, interval, 0), std::invalid_argument);
}
