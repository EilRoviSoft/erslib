// doctest
#include <doctest/doctest.h>

// std
#include <array>
#include <chrono>
#include <thread>
#include <vector>

// ers
#include <erslib/core/random.hpp>
#include <erslib/core/thread_safe/shuffle_seed.hpp>
#include <erslib/core/type/general.hpp>


using namespace std::chrono_literals;


namespace {
    constexpr auto lifetime = 30min;

    ers::timestamp_t t0() {
        return ers::timestamp_t() + 24h;
    }
}


TEST_CASE("shuffle seeds: stable for the whole lifetime") {
    ers::thread_safe::ShuffleSeeds seeds(lifetime);

    const auto first = seeds.acquire("session-a", t0());

    CHECK(seeds.acquire("session-a", t0()) == first);
    CHECK(seeds.acquire("session-a", t0() + 29min) == first);
}

TEST_CASE("shuffle seeds: sessions do not share an order") {
    ers::thread_safe::ShuffleSeeds seeds(lifetime);

    CHECK(seeds.acquire("session-a", t0()) != seeds.acquire("session-b", t0()));
}

TEST_CASE("shuffle seeds: the order changes once the lifetime is over") {
    ers::thread_safe::ShuffleSeeds seeds(lifetime);

    const auto before = seeds.acquire("session-a", t0());

    CHECK(seeds.acquire("session-a", t0() + lifetime) != before);
}

TEST_CASE("shuffle seeds: reading does not extend the lifetime") {
    ers::thread_safe::ShuffleSeeds seeds(lifetime);

    const auto first = seeds.acquire("session-a", t0());

    for (auto at = t0(); at < t0() + lifetime; at += 5min)
        CHECK(seeds.acquire("session-a", at) == first);

    CHECK(seeds.acquire("session-a", t0() + lifetime) != first);
}

TEST_CASE("shuffle seeds: rotate replaces the order and restarts the clock") {
    ers::thread_safe::ShuffleSeeds seeds(lifetime);

    const auto first = seeds.acquire("session-a", t0());
    const auto rotated = seeds.rotate("session-a", t0() + 20min);

    CHECK(rotated != first);
    CHECK(seeds.acquire("session-a", t0() + 20min) == rotated);
    CHECK(seeds.acquire("session-a", t0() + 45min) == rotated);
}

TEST_CASE("shuffle seeds: forget drops the entry") {
    ers::thread_safe::ShuffleSeeds seeds(lifetime);

    const auto first = seeds.acquire("session-a", t0());
    seeds.forget("session-a");

    CHECK(seeds.size() == 0);
    CHECK(seeds.acquire("session-a", t0()) != first);
}

TEST_CASE("shuffle seeds: maintain evicts only what expired") {
    ers::thread_safe::ShuffleSeeds seeds(lifetime);

    seeds.acquire("old", t0());
    const auto fresh = seeds.acquire("fresh", t0() + 20min);

    CHECK(seeds.size() == 2);
    CHECK(seeds.maintain(t0() + lifetime) == 1);
    CHECK(seeds.size() == 1);
    CHECK(seeds.acquire("fresh", t0() + 21min) == fresh);
}

TEST_CASE("shuffle seeds: an explicit master seed reproduces the sequence") {
    ers::thread_safe::ShuffleSeeds one(lifetime, 0xC0FFEE);
    ers::thread_safe::ShuffleSeeds two(lifetime, 0xC0FFEE);

    for (const auto* it : { "a", "b", "c" })
        CHECK(one.acquire(it, t0()) == two.acquire(it, t0()));

    ers::thread_safe::ShuffleSeeds other(lifetime, 0xBADF00D);
    CHECK(other.acquire("a", t0()) != one.acquire("a", t0()));
}

TEST_CASE("shuffle seeds: concurrent acquire agrees on one value") {
    ers::thread_safe::ShuffleSeeds seeds(lifetime);

    constexpr size_t workers = 8;
    constexpr size_t rounds = 500;

    std::array<std::vector<int64_t>, workers> seen;
    std::vector<std::thread> threads;

    for (size_t w = 0; w < workers; w++) {
        threads.emplace_back([&, w] {
            seen[w].reserve(rounds);

            for (size_t i = 0; i < rounds; i++)
                seen[w].emplace_back(seeds.acquire("hot"));
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

TEST_CASE("entropy: varies and fills all 64 bits") {
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

