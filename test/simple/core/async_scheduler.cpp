// catch2
#include <catch2/catch_test_macros.hpp>

// std
#include <atomic>
#include <chrono>
#include <future>
#include <mutex>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

// boost
#include <boost/asio.hpp>

// ers
#include <erslib/core/async/scheduler.hpp>

// test
#include "support/gate.hpp"
#include "support/test_case.hpp"


namespace asio = boost::asio;
using namespace std::chrono_literals;


// Spawning --------------------------------------------------------------------

namespace {
    std::atomic destroyed = 0;
    std::atomic started = 0;
    std::mutex trace_mutex;
    std::string trace;


    struct Guard {
        ~Guard() { ++destroyed; }
    };


    void note(char name) {
        std::scoped_lock lock(trace_mutex);
        trace += name;
    }


    ers::Task<int> answer() {
        co_return 42;
    }

    ers::Task<int> broken() {
        co_return ers::make_error("boom");
    }

    ers::Task<void> blocker(std::shared_future<void> gate) {
        gate.wait();
        co_return ers::ok;
    }

    ers::Task<void> steps(char name) {
        auto executor = co_await asio::this_coro::executor;

        for (int i = 0; i < 3; ++i) {
            note(name);
            co_await asio::post(executor, asio::use_awaitable);
        }

        co_return ers::ok;
    }

    ers::Task<int> inner() {
        auto executor = co_await asio::this_coro::executor;
        co_await asio::post(executor, asio::use_awaitable);
        co_return 7;
    }

    ers::Task<int> outer() {
        ERS_CO_TRY_AS(int value, co_await inner());
        co_return value + 1;
    }

    ers::Task<int> forever() {
        Guard guard;
        asio::steady_timer timer(co_await asio::this_coro::executor, 1h);

        ++started;
        co_await timer.async_wait(asio::as_tuple(asio::use_awaitable));

        co_return 0;
    }
}


ERS_TEST_CASE("spawn", "[async]") {
    ers::Scheduler scheduler({ .threads = 1 });

    auto result = scheduler.spawn(answer()).get();

    REQUIRE(result.has_value());
    REQUIRE(*result == 42);
}

ERS_TEST_CASE("error", "[async]") {
    ers::Scheduler scheduler({ .threads = 1 });

    auto result = scheduler.spawn(broken()).get();

    REQUIRE(!result);
    REQUIRE(result.error().message() == "boom");
}

ERS_TEST_CASE("yield", "[async]") {
    trace.clear();

    ers::Scheduler scheduler({ .threads = 1 });
    test::Gate gate;

    auto blocked = scheduler.spawn(blocker(gate.future()));
    auto first = scheduler.spawn(steps('A'));
    auto second = scheduler.spawn(steps('B'));

    gate.open();

    REQUIRE(blocked.get().has_value());
    REQUIRE(first.get().has_value());
    REQUIRE(second.get().has_value());
    REQUIRE(trace == "ABABAB");
}

ERS_TEST_CASE("nested", "[async]") {
    ers::Scheduler scheduler({ .threads = 1 });

    auto result = scheduler.spawn(outer()).get();

    REQUIRE(result.has_value());
    REQUIRE(*result == 8);
}

ERS_TEST_CASE("jobs", "[async]") {
    ers::Scheduler scheduler({ .threads = 2 });
    std::vector<ers::Job<int>> jobs;

    for (int i = 0; i < 100; ++i)
        jobs.emplace_back(scheduler.spawn(outer()));

    for (auto& job : jobs) {
        auto result = job.get();

        REQUIRE(result.has_value());
        REQUIRE(*result == 8);
    }
}

ERS_TEST_CASE("stop", "[async]") {
    destroyed = 0;
    started = 0;

    ers::Job<int> job;

    {
        ers::Scheduler scheduler({ .threads = 1 });
        job = scheduler.spawn(forever());

        while (started < 1)
            std::this_thread::yield();

        scheduler.stop();

        REQUIRE(job.wait_for(50ms) == std::future_status::timeout);
    }

    REQUIRE(job.wait_for(0s) == std::future_status::ready);
    REQUIRE(!job.get());
    REQUIRE(destroyed == 1);
}

ERS_TEST_CASE("late", "[async]") {
    ers::Scheduler scheduler({ .threads = 1 });
    scheduler.stop();

    REQUIRE(!scheduler.spawn(answer()).get());
}

ERS_TEST_CASE("drop", "[async]") {
    ers::Job<int> job;

    {
        ers::Scheduler scheduler({ .threads = 1 });
        job = scheduler.spawn(forever());
    }

    REQUIRE(job.wait_for(0s) == std::future_status::ready);
    REQUIRE(!job.get());
}


// Blocking --------------------------------------------------------------------

namespace {
    struct Threads {
        std::thread::id before;
        std::thread::id inside;
        std::thread::id after;
    };


    Threads threads;
    std::atomic running = 0;
    std::atomic peak = 0;


    ers::Task<int> hop(ers::Scheduler& scheduler) {
        threads.before = std::this_thread::get_id();

        ERS_CO_TRY_AS(int value, co_await scheduler.blocking([]() -> ers::Result<int> {
            threads.inside = std::this_thread::get_id();
            return 11;
        }));

        threads.after = std::this_thread::get_id();
        co_return value + 1;
    }

    ers::Task<int> failing(ers::Scheduler& scheduler) {
        ERS_CO_TRY_AS(int value, co_await scheduler.blocking([]() -> ers::Result<int> {
            return ers::make_error("db down");
        }));
        co_return value;
    }

    ers::Task<int> throwing(ers::Scheduler& scheduler) {
        ERS_CO_TRY_AS(int value, co_await scheduler.blocking([]() -> ers::Result<int> {
            throw std::runtime_error("libpq");
        }));
        co_return value;
    }

    ers::Task<int> waiting(ers::Scheduler& scheduler, std::shared_future<void> gate) {
        ERS_CO_TRY_AS(int value, co_await scheduler.blocking([gate]() -> ers::Result<int> {
            int now = ++running;
            int seen = peak.load();
            while (now > seen && !peak.compare_exchange_weak(seen, now)) {}

            gate.wait();
            --running;
            return 1;
        }));
        co_return value;
    }

    ers::Task<void> ticker(std::atomic<int>& count) {
        for (int i = 0; i < 3; ++i) {
            ++count;
            co_await ers::yield();
        }

        co_return ers::ok;
    }
}


ERS_TEST_CASE("hop", "[async]") {
    ers::Scheduler scheduler({ .threads = 1, .blocking_threads = 1 });

    auto result = scheduler.spawn(hop(scheduler)).get();

    REQUIRE(result.has_value());
    REQUIRE(*result == 12);
    REQUIRE(threads.inside != threads.before);
    REQUIRE(threads.after == threads.before);
}

ERS_TEST_CASE("blocking", "[async]") {
    ers::Scheduler scheduler({ .threads = 1, .blocking_threads = 1 });

    auto failed = scheduler.spawn(failing(scheduler)).get();
    REQUIRE(!failed);
    REQUIRE(failed.error().message() == "db down");

    auto thrown = scheduler.spawn(throwing(scheduler)).get();
    REQUIRE(!thrown);
    REQUIRE(thrown.error().message().find("libpq") != std::string_view::npos);
}

ERS_TEST_CASE("free", "[async]") {
    ers::Scheduler scheduler({ .threads = 1, .blocking_threads = 1 });
    test::Gate gate;

    auto slow = scheduler.spawn(waiting(scheduler, gate.future()));
    auto fast = scheduler.spawn(answer()).get();

    REQUIRE(fast.has_value());
    REQUIRE(slow.wait_for(0s) == std::future_status::timeout);

    gate.open();
    REQUIRE(slow.get().has_value());
}

ERS_TEST_CASE("cap", "[async]") {
    peak = 0;
    running = 0;

    ers::Scheduler scheduler({ .threads = 1, .blocking_threads = 2 });
    test::Gate gate;
    std::vector<ers::Job<int>> jobs;

    REQUIRE(scheduler.options().capacity() == 3);

    for (int i = 0; i < 3; ++i)
        jobs.emplace_back(scheduler.spawn(waiting(scheduler, gate.future())));

    gate.open();

    for (auto& job : jobs)
        REQUIRE(job.get().has_value());

    REQUIRE(peak <= 2);
}

ERS_TEST_CASE("tick", "[async]") {
    std::atomic count = 0;
    ers::Scheduler scheduler({ .threads = 1 });

    REQUIRE(scheduler.spawn(ticker(count)).get().has_value());
    REQUIRE(count == 3);
}
