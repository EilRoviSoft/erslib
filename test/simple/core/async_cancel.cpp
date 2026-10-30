// catch2
#include <catch2/catch_test_macros.hpp>

// std
#include <atomic>
#include <chrono>
#include <future>
#include <string_view>
#include <thread>

// boost
#include <boost/asio.hpp>

// ers
#include <erslib/core/async/bulkhead.hpp>

// test
#include "support/gate.hpp"
#include "support/test_case.hpp"


// Details ---------------------------------------------------------------------

namespace {
    namespace asio = boost::asio;
    using namespace std::chrono_literals;

    std::atomic destroyed = 0;
    std::atomic started = 0;
    std::atomic ran = 0;
    std::atomic turns = 0;


    struct Guard {
        ~Guard() { ++destroyed; }
    };


    void reset() {
        destroyed = 0;
        started = 0;
        ran = 0;
        turns = 0;
    }


    bool mentions(const ers::Result<int>& result, std::string_view text) {
        return !result && result.error().message().find(text) != std::string_view::npos;
    }


    ers::AsyncTask<int> sleeping() {
        Guard guard;
        asio::steady_timer timer(co_await asio::this_coro::executor, 1h);

        ++started;
        co_await timer.async_wait(asio::use_awaitable);

        co_return 0;
    }

    ers::AsyncTask<int> quiet() {
        Guard guard;
        asio::steady_timer timer(co_await asio::this_coro::executor, 1h);

        ++started;
        auto [ec] = co_await timer.async_wait(asio::as_tuple(asio::use_awaitable));

        if (ec)
            co_return ers::make_error("woke up: {}", ec.message());

        co_return 0;
    }

    ers::AsyncTask<int> holding(std::shared_future<void> gate) {
        ++started;
        gate.wait();

        co_return 1;
    }

    ers::AsyncTask<int> counted() {
        ++ran;
        co_return 7;
    }

    ers::AsyncTask<int> stepping() {
        Guard guard;

        while (true) {
            ++turns;
            ERS_CO_TRY(co_await ers::checkpoint());
        }
    }

    ers::AsyncTask<int> answer() {
        co_return 42;
    }
}


// Cancellation ----------------------------------------------------------------

ERS_TEST_CASE("cancel", "[async]") {
    reset();

    ers::Scheduler scheduler({ .threads = 2 });

    auto job = scheduler.spawn(sleeping(), { .interruptible = true });

    while (started < 1)
        std::this_thread::yield();

    REQUIRE(job.cancel());
    REQUIRE(job.cancelled());

    auto result = job.get();

    REQUIRE(mentions(result, "cancelled"));
    REQUIRE(destroyed == 1);
}

ERS_TEST_CASE("quiet", "[async]") {
    reset();

    ers::Scheduler scheduler({ .threads = 2 });

    auto job = scheduler.spawn(quiet(), { .interruptible = true });

    while (started < 1)
        std::this_thread::yield();

    REQUIRE(job.cancel());

    REQUIRE(mentions(job.get(), "woke up"));
    REQUIRE(destroyed == 1);
}

ERS_TEST_CASE("pinned", "[async]") {
    reset();

    ers::Scheduler scheduler({ .threads = 1 });
    test::Gate gate;

    auto job = scheduler.spawn(holding(gate.future()));

    REQUIRE(!job.cancel());
    REQUIRE(!job.cancelled());

    gate.open();

    auto result = job.get();

    REQUIRE(result.has_value());
    REQUIRE(*result == 1);
}

ERS_TEST_CASE("step", "[async]") {
    reset();

    ers::Scheduler scheduler({ .threads = 2 });

    auto job = scheduler.spawn(stepping(), { .interruptible = true });

    while (turns < 3)
        std::this_thread::yield();

    REQUIRE(job.cancel());

    REQUIRE(mentions(job.get(), "cancelled"));
    REQUIRE(destroyed == 1);
}

ERS_TEST_CASE("early", "[async]") {
    reset();

    ers::Scheduler scheduler({ .threads = 2 });
    ers::Bulkhead bulkhead(scheduler, { .max_running = 1, .max_waiting = 2 });
    test::Gate gate;

    auto first = bulkhead.spawn(holding(gate.future()));
    auto second = bulkhead.spawn(counted(), { .interruptible = true });

    REQUIRE(bulkhead.waiting() == 1);
    REQUIRE(second.cancel());

    gate.open();

    REQUIRE(first.get().has_value());
    REQUIRE(mentions(second.get(), "cancelled"));
    REQUIRE(ran == 0);
    REQUIRE(bulkhead.running() == 0);
}

ERS_TEST_CASE("slot", "[async]") {
    reset();

    ers::Scheduler scheduler({ .threads = 2 });
    ers::Bulkhead bulkhead(scheduler, { .max_running = 1, .max_waiting = 1 });

    auto first = bulkhead.spawn(sleeping(), { .interruptible = true });
    auto second = bulkhead.spawn(answer());

    while (started < 1)
        std::this_thread::yield();

    REQUIRE(first.cancel());

    REQUIRE(mentions(first.get(), "cancelled"));

    auto result = second.get();

    REQUIRE(result.has_value());
    REQUIRE(*result == 42);
    REQUIRE(bulkhead.running() == 0);
}

ERS_TEST_CASE("refused", "[async]") {
    reset();

    ers::Scheduler scheduler({ .threads = 1 });
    ers::Bulkhead bulkhead(scheduler, { .max_running = 1, .max_waiting = 0 });
    test::Gate gate;

    auto first = bulkhead.spawn(holding(gate.future()));
    auto second = bulkhead.spawn(counted(), { .interruptible = true });

    REQUIRE(second.rejected());
    REQUIRE(!second.cancel());

    gate.open();
    REQUIRE(first.get().has_value());
}


// Stopping --------------------------------------------------------------------

ERS_TEST_CASE("stop", "[async]") {
    reset();

    ers::Scheduler scheduler({ .threads = 2 });

    auto job = scheduler.spawn(sleeping(), { .interruptible = true });

    while (started < 1)
        std::this_thread::yield();

    scheduler.stop(5s);

    REQUIRE(job.ready());
    REQUIRE(mentions(job.get(), "cancelled"));
    REQUIRE(destroyed == 1);
}

ERS_TEST_CASE("grace", "[async]") {
    reset();

    ers::AsyncJob<int> job;
    std::chrono::steady_clock::duration took;

    {
        ers::Scheduler scheduler({ .threads = 2 });

        job = scheduler.spawn(sleeping());

        while (started < 1)
            std::this_thread::yield();

        auto begin = std::chrono::steady_clock::now();
        scheduler.stop(50ms);
        took = std::chrono::steady_clock::now() - begin;

        REQUIRE(!job.ready());
    }

    REQUIRE(took >= 50ms);
    REQUIRE(job.ready());
    REQUIRE(!job.get());
    REQUIRE(destroyed == 1);
}

ERS_TEST_CASE("queued", "[async]") {
    reset();

    ers::AsyncJob<int> active;
    ers::AsyncJob<int> waiting;

    {
        ers::Scheduler scheduler({ .threads = 2 });
        ers::Bulkhead bulkhead(scheduler, { .max_running = 1, .max_waiting = 2 });

        active = bulkhead.spawn(sleeping(), { .interruptible = true });
        waiting = bulkhead.spawn(counted(), { .interruptible = true });

        while (started < 1)
            std::this_thread::yield();

        scheduler.stop(5s);

        REQUIRE(active.ready());
        REQUIRE(waiting.ready());
    }

    REQUIRE(mentions(active.get(), "cancelled"));
    REQUIRE(mentions(waiting.get(), "cancelled"));
    REQUIRE(ran == 0);
}
