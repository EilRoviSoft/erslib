// catch2
#include <catch2/catch_test_macros.hpp>

// std
#include <atomic>
#include <chrono>
#include <future>
#include <mutex>
#include <stdexcept>
#include <string>
#include <string_view>
#include <thread>
#include <vector>

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

    std::atomic running = 0;
    std::atomic peak = 0;
    std::atomic started = 0;
    std::atomic destroyed = 0;
    std::mutex order_mutex;
    std::string order;


    struct Guard {
        ~Guard() { ++destroyed; }
    };


    void reset() {
        running = 0;
        peak = 0;
        started = 0;
        destroyed = 0;
        order.clear();
    }


    ers::AsyncTask<int> holding(std::shared_future<void> gate, int id) {
        {
            std::scoped_lock lock(order_mutex);
            order += static_cast<char>('0' + id);
        }

        int now = ++running;
        int seen = peak.load();
        while (now > seen && !peak.compare_exchange_weak(seen, now)) {}
        ++started;

        gate.wait();

        --running;
        co_return id;
    }

    ers::AsyncTask<int> failing() {
        co_return ers::make_error("boom");
    }

    ers::AsyncTask<int> throwing() {
        throw std::runtime_error("kaput");
        co_return 0;
    }

    ers::AsyncTask<int> answer() {
        co_return 42;
    }

    ers::AsyncTask<int> forever() {
        Guard guard;
        asio::steady_timer timer(co_await asio::this_coro::executor, 1h);

        ++started;
        co_await timer.async_wait(asio::as_tuple(asio::use_awaitable));

        co_return 0;
    }
}


// Admission -------------------------------------------------------------------

ERS_TEST_CASE("limit", "[async]") {
    reset();

    ers::Scheduler scheduler({ .threads = 2 });
    ers::Bulkhead group(scheduler, { .max_running = 2, .max_waiting = 10 });
    test::Gate gate;
    std::vector<ers::AsyncJob<int>> jobs;

    for (int i = 0; i < 6; ++i) {
        auto job = group.spawn(holding(gate.future(), i));

        REQUIRE(!job.rejected());
        jobs.emplace_back(std::move(job));
    }

    REQUIRE(group.running() == 2);
    REQUIRE(group.waiting() == 4);

    gate.open();

    for (auto& job : jobs)
        REQUIRE(job.get().has_value());

    REQUIRE(peak <= 2);
    REQUIRE(group.running() == 0);
    REQUIRE(group.waiting() == 0);
}

ERS_TEST_CASE("full", "[async]") {
    reset();

    ers::Scheduler scheduler({ .threads = 1 });
    ers::Bulkhead group(scheduler, { .max_running = 1, .max_waiting = 1 });
    test::Gate gate;

    auto first = group.spawn(holding(gate.future(), 0));
    auto second = group.spawn(holding(gate.future(), 1));
    auto third = group.spawn(holding(gate.future(), 2));

    REQUIRE(!first.rejected());
    REQUIRE(!second.rejected());
    REQUIRE(third.rejected());
    REQUIRE(third.get().error().message() == "The bulkhead is full.");

    gate.open();

    REQUIRE(first.get().has_value());
    REQUIRE(second.get().has_value());

    auto again = group.spawn(answer());
    REQUIRE(!again.rejected());
    REQUIRE(again.get().has_value());
}

ERS_TEST_CASE("fifo", "[async]") {
    reset();

    ers::Scheduler scheduler({ .threads = 1 });
    ers::Bulkhead group(scheduler, { .max_running = 1, .max_waiting = 10 });
    test::Gate gate;
    std::vector<ers::AsyncJob<int>> jobs;

    for (int i = 0; i < 5; ++i)
        jobs.emplace_back(group.spawn(holding(gate.future(), i)));

    gate.open();

    for (auto& job : jobs)
        REQUIRE(job.get().has_value());

    REQUIRE(order == "01234");
}

ERS_TEST_CASE("stopped", "[async]") {
    ers::Scheduler scheduler({ .threads = 1 });
    scheduler.stop();

    ers::Bulkhead group(scheduler, { .max_running = 1, .max_waiting = 1 });

    REQUIRE(group.spawn(answer()).rejected());
}


// Release ---------------------------------------------------------------------

ERS_TEST_CASE("release", "[async]") {
    ers::Scheduler scheduler({ .threads = 2 });
    ers::Bulkhead group(scheduler, { .max_running = 1, .max_waiting = 4 });

    auto failed = group.spawn(failing());
    auto thrown = group.spawn(throwing());
    auto fine = group.spawn(answer());

    REQUIRE(!failed.get());

    auto error = thrown.get();
    REQUIRE(!error);
    REQUIRE(error.error().message().find("kaput") != std::string_view::npos);

    auto result = fine.get();
    REQUIRE(result.has_value());
    REQUIRE(*result == 42);

    REQUIRE(group.running() == 0);
}


// Teardown --------------------------------------------------------------------

ERS_TEST_CASE("teardown", "[async]") {
    reset();

    ers::AsyncJob<int> active;
    ers::AsyncJob<int> first;
    ers::AsyncJob<int> second;

    {
        ers::Scheduler scheduler({ .threads = 2 });
        ers::Bulkhead group(scheduler, { .max_running = 1, .max_waiting = 4 });

        active = group.spawn(forever());

        while (started < 1)
            std::this_thread::yield();

        first = group.spawn(forever());
        second = group.spawn(forever());

        REQUIRE(group.waiting() == 2);
    }

    REQUIRE(active.wait_for(0s) == std::future_status::ready);
    REQUIRE(first.wait_for(0s) == std::future_status::ready);
    REQUIRE(second.wait_for(0s) == std::future_status::ready);

    REQUIRE(!active.get());
    REQUIRE(!first.get());
    REQUIRE(!second.get());

    REQUIRE(started == 1);
    REQUIRE(destroyed == 1);
}

ERS_TEST_CASE("owner", "[async]") {
    reset();

    ers::Scheduler scheduler({ .threads = 1 });
    test::Gate gate;
    ers::AsyncJob<int> active;
    ers::AsyncJob<int> queued;

    {
        ers::Bulkhead group(scheduler, { .max_running = 1, .max_waiting = 2 });

        active = group.spawn(holding(gate.future(), 0));
        queued = group.spawn(holding(gate.future(), 1));
    }

    gate.open();

    REQUIRE(active.get().has_value());
    REQUIRE(queued.wait_for(0s) == std::future_status::ready);
    REQUIRE(!queued.get());
}

ERS_TEST_CASE("clear", "[async]") {
    reset();

    ers::Scheduler scheduler({ .threads = 1 });
    ers::Bulkhead group(scheduler, { .max_running = 1, .max_waiting = 3 });
    test::Gate gate;

    auto active = group.spawn(holding(gate.future(), 0));
    auto first = group.spawn(holding(gate.future(), 1));
    auto second = group.spawn(holding(gate.future(), 2));

    group.clear();

    REQUIRE(group.waiting() == 0);
    REQUIRE(first.wait_for(0s) == std::future_status::ready);
    REQUIRE(second.wait_for(0s) == std::future_status::ready);
    REQUIRE(!first.get());
    REQUIRE(!second.get());

    gate.open();

    REQUIRE(active.get().has_value());
    REQUIRE(group.running() == 0);
}
