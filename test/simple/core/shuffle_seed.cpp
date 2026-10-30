// catch2
#include <catch2/catch_test_macros.hpp>

// std
#include <array>
#include <chrono>
#include <thread>
#include <vector>

// ers
#include <erslib/core/random.hpp>
#include <erslib/core/thread_safe/shuffle_seed.hpp>
#include <erslib/core/type/general.hpp>

// test
#include "support/test_case.hpp"


using namespace std::chrono_literals;


namespace {
    constexpr auto lifetime = 30min;

    ers::timestamp_t t0() {
        return ers::timestamp_t() + 24h;
    }
}


ERS_TEST_CASE("stable", "[core][thread_safe]") {
    ers::thread_safe::ShuffleSeeds seeds(lifetime);

    const auto first = seeds.acquire("session-a", t0());

    CHECK(seeds.acquire("session-a", t0()) == first);
    CHECK(seeds.acquire("session-a", t0() + 29min) == first);
}

ERS_TEST_CASE("sessions", "[core][thread_safe]") {
    ers::thread_safe::ShuffleSeeds seeds(lifetime);

    CHECK(seeds.acquire("session-a", t0()) != seeds.acquire("session-b", t0()));
}

ERS_TEST_CASE("expiry", "[core][thread_safe]") {
    ers::thread_safe::ShuffleSeeds seeds(lifetime);

    const auto before = seeds.acquire("session-a", t0());

    CHECK(seeds.acquire("session-a", t0() + lifetime) != before);
}

ERS_TEST_CASE("read", "[core][thread_safe]") {
    ers::thread_safe::ShuffleSeeds seeds(lifetime);

    const auto first = seeds.acquire("session-a", t0());

    for (auto at = t0(); at < t0() + lifetime; at += 5min)
        CHECK(seeds.acquire("session-a", at) == first);

    CHECK(seeds.acquire("session-a", t0() + lifetime) != first);
}

ERS_TEST_CASE("rotate", "[core][thread_safe]") {
    ers::thread_safe::ShuffleSeeds seeds(lifetime);

    const auto first = seeds.acquire("session-a", t0());
    const auto rotated = seeds.rotate("session-a", t0() + 20min);

    CHECK(rotated != first);
    CHECK(seeds.acquire("session-a", t0() + 20min) == rotated);
    CHECK(seeds.acquire("session-a", t0() + 45min) == rotated);
}

ERS_TEST_CASE("forget", "[core][thread_safe]") {
    ers::thread_safe::ShuffleSeeds seeds(lifetime);

    const auto first = seeds.acquire("session-a", t0());
    seeds.forget("session-a");

    CHECK(seeds.size() == 0);
    CHECK(seeds.acquire("session-a", t0()) != first);
}

ERS_TEST_CASE("maintain", "[core][thread_safe]") {
    ers::thread_safe::ShuffleSeeds seeds(lifetime);

    seeds.acquire("old", t0());
    const auto fresh = seeds.acquire("fresh", t0() + 20min);

    CHECK(seeds.size() == 2);
    CHECK(seeds.maintain(t0() + lifetime) == 1);
    CHECK(seeds.size() == 1);
    CHECK(seeds.acquire("fresh", t0() + 21min) == fresh);
}

ERS_TEST_CASE("master seed", "[core][thread_safe]") {
    ers::thread_safe::ShuffleSeeds one(lifetime, 0xC0FFEE);
    ers::thread_safe::ShuffleSeeds two(lifetime, 0xC0FFEE);

    for (const auto* it : { "a", "b", "c" })
        CHECK(one.acquire(it, t0()) == two.acquire(it, t0()));

    ers::thread_safe::ShuffleSeeds other(lifetime, 0xBADF00D);
    CHECK(other.acquire("a", t0()) != one.acquire("a", t0()));
}

ERS_TEST_CASE("concurrent", "[core][thread_safe][concurrency][slow]") {
    ers::thread_safe::ShuffleSeeds seeds(lifetime);

    constexpr size_t workers = 8;
    constexpr size_t rounds = 500;

    std::array<std::vector<int64_t>, workers> seen;
    std::vector<std::thread> threads;

    for (size_t w = 0; w < workers; w++) {
        threads.push_back([&, w] {
            seen[w].reserve(rounds);

            for (size_t i = 0; i < rounds; i++)
                seen[w].push_back(seeds.acquire("hot"));
        });
    }

    for (auto& it : threads)
        it.join();

    const int64_t expected = seen.front().front();

    for (const auto& list : seen) {
        for (const int64_t it : list)
            CHECK(it == expected);
    }

    CHECK(seeds.size() == 1);
}

ERS_TEST_CASE("entropy", "[core][thread_safe]") {
    const auto first = ers::random::entropy();

    u64 combined = 0;
    bool varied = false;

    for (int i = 0; i < 64; i++) {
        const auto it = ers::random::entropy();

        combined |= it;
        varied = varied || it != first;
    }

    CHECK(varied);
    CHECK((combined >> 32) != 0);
}

