// catch2
#include <catch2/catch_test_macros.hpp>

// std
#include <atomic>
#include <chrono>
#include <mutex>
#include <string>
#include <string_view>
#include <thread>

// boost
#include <boost/asio.hpp>

// ers
#include <erslib/core/async/runtime.hpp>

// test
#include "support/test_case.hpp"


// Details ---------------------------------------------------------------------

namespace {
    namespace asio = boost::asio;
    using namespace std::chrono_literals;

    using clock_type = std::chrono::steady_clock;


    class Recorder {
    public:
        void note(char c) {
            std::scoped_lock lock(_mutex);
            _trace += c;
        }

        std::string trace() {
            std::scoped_lock lock(_mutex);
            return _trace;
        }


    private:
        std::mutex _mutex;
        std::string _trace;
    };


    class Probe : public ers::IService {
    public:
        Probe(Recorder& recorder, char id, bool fail = false) :
            _recorder(recorder),
            _id(id),
            _name(1, id),
            _fail(fail) {
        }

        std::string_view name() const override { return _name; }

        ers::Result<void> start() override {
            _recorder.note(_id);

            if (_fail)
                return ers::make_error("refused");

            return ers::ok;
        }

        void stop() override {
            _recorder.note(static_cast<char>(_id - 'a' + 'A'));
        }


    private:
        Recorder& _recorder;
        char _id;
        std::string _name;
        bool _fail;
    };


    ers::AsyncTask<int> answer() {
        co_return 42;
    }


    class Converter : public ers::IService, public ers::IWorker {
    public:
        std::string_view name() const override { return "converter"; }

        void declare(ers::AsyncRegistry& registry) override {
            _bulkhead = &registry.bulkhead({ .max_running = 1, .max_waiting = 0 });
        }

        ers::Result<void> start() override {
            if (!_bulkhead)
                return ers::make_error("declare was not called before start");

            return ers::ok;
        }

        ers::AsyncJob<int> convert() {
            return _bulkhead->spawn(answer());
        }


    private:
        ers::Bulkhead* _bulkhead = nullptr;
    };


    class Ticker : public ers::IService, public ers::IWorker {
    public:
        Ticker(std::chrono::milliseconds interval, ers::periodic_options_t options = {}) :
            _interval(interval),
            _options(std::move(options)) {
        }

        std::string_view name() const override { return "ticker"; }

        void declare(ers::AsyncRegistry& registry) override {
            registry.every(_interval, [this] { return tick(); }, _options);
        }

        ers::AsyncTask<void> tick() {
            ++count;

            if (fail)
                co_return ers::make_error("tick failed");

            co_return ers::ok;
        }


        std::atomic<int> count = 0;
        bool fail = false;


    private:
        std::chrono::milliseconds _interval;
        ers::periodic_options_t _options;
    };


    class Sleeper : public ers::IService, public ers::IWorker {
    public:
        std::string_view name() const override { return "sleeper"; }

        void declare(ers::AsyncRegistry& registry) override {
            registry.every(1ms, [this] { return nap(); }, { .immediate = true });
        }

        ers::AsyncTask<void> nap() {
            asio::steady_timer timer(co_await asio::this_coro::executor, 1h);

            ++entered;
            co_await timer.async_wait(asio::use_awaitable);

            co_return ers::ok;
        }


        std::atomic<int> entered = 0;
    };


    template<typename Predicate>
    bool wait_until(Predicate predicate) {
        auto deadline = clock_type::now() + 5s;

        while (!predicate()) {
            if (clock_type::now() > deadline)
                return false;

            std::this_thread::sleep_for(2ms);
        }

        return true;
    }
}


// Lifecycle -------------------------------------------------------------------

ERS_TEST_CASE("order", "[async]") {
    Recorder recorder;

    {
        ers::AsyncRuntime runtime({ .threads = 2 });

        runtime.add<Probe>(recorder, 'a');
        runtime.add<Probe>(recorder, 'b');
        runtime.add<Probe>(recorder, 'c');

        REQUIRE(runtime.start().has_value());
        REQUIRE(recorder.trace() == "abc");

        runtime.stop();
    }

    REQUIRE(recorder.trace() == "abcCBA");
}

ERS_TEST_CASE("fail", "[async]") {
    Recorder recorder;
    ers::AsyncRuntime runtime({ .threads = 2 });

    runtime.add<Probe>(recorder, 'a');
    runtime.add<Probe>(recorder, 'b', true);
    runtime.add<Probe>(recorder, 'c');

    auto result = runtime.start();

    REQUIRE(!result);
    REQUIRE(result.error().message().find("'b'") != std::string_view::npos);
    REQUIRE(result.error().message().find("refused") != std::string_view::npos);
    REQUIRE(recorder.trace() == "abA");
}

ERS_TEST_CASE("twice", "[async]") {
    Recorder recorder;
    ers::AsyncRuntime runtime({ .threads = 2 });

    runtime.add<Probe>(recorder, 'a');

    REQUIRE(runtime.start().has_value());
    REQUIRE(!runtime.start());

    runtime.stop();
    runtime.stop();

    REQUIRE(recorder.trace() == "aA");
}

ERS_TEST_CASE("idle", "[async]") {
    Recorder recorder;

    {
        ers::AsyncRuntime runtime({ .threads = 1 });
        runtime.add<Probe>(recorder, 'a');
        runtime.stop();
    }

    REQUIRE(recorder.trace().empty());
}

ERS_TEST_CASE("drop", "[async]") {
    Recorder recorder;

    {
        ers::AsyncRuntime runtime({ .threads = 2 });
        runtime.add<Probe>(recorder, 'a');

        REQUIRE(runtime.start().has_value());
    }

    REQUIRE(recorder.trace() == "aA");
}


// Workers ---------------------------------------------------------------------

ERS_TEST_CASE("declare", "[async]") {
    ers::AsyncRuntime runtime({ .threads = 2 });
    auto& converter = runtime.add<Converter>();

    REQUIRE(runtime.start().has_value());

    auto job = converter.convert();
    REQUIRE(!job.rejected());

    auto result = job.get();

    REQUIRE(result.has_value());
    REQUIRE(*result == 42);
}


// Periodic --------------------------------------------------------------------

ERS_TEST_CASE("every", "[async]") {
    ers::AsyncRuntime runtime({ .threads = 2 });
    auto& ticker = runtime.add<Ticker>(5ms);

    REQUIRE(runtime.start().has_value());
    REQUIRE(wait_until([&] { return ticker.count >= 3; }));

    runtime.stop();

    int stopped = ticker.count;
    std::this_thread::sleep_for(100ms);

    REQUIRE(ticker.count == stopped);
}

ERS_TEST_CASE("now", "[async]") {
    ers::AsyncRuntime runtime({ .threads = 2 });
    auto& ticker = runtime.add<Ticker>(1h, ers::periodic_options_t{ .immediate = true });

    REQUIRE(runtime.start().has_value());
    REQUIRE(wait_until([&] { return ticker.count >= 1; }));
}

ERS_TEST_CASE("later", "[async]") {
    ers::AsyncRuntime runtime({ .threads = 2 });
    auto& ticker = runtime.add<Ticker>(1h);

    REQUIRE(runtime.start().has_value());

    std::this_thread::sleep_for(50ms);

    REQUIRE(ticker.count == 0);
}

ERS_TEST_CASE("report", "[async]") {
    std::atomic reported = 0;

    ers::AsyncRuntime runtime({ .threads = 2 });
    auto& ticker = runtime.add<Ticker>(2ms, ers::periodic_options_t{
        .name = "flaky",
        .on_error = [&](const ers::Diagnostic&) { ++reported; },
        });

    ticker.fail = true;

    REQUIRE(runtime.start().has_value());
    REQUIRE(wait_until([&] { return reported >= 3; }));
    REQUIRE(ticker.count >= 3);
}

ERS_TEST_CASE("halt", "[async]") {
    ers::AsyncRuntime runtime({ .threads = 2 });
    auto& sleeper = runtime.add<Sleeper>();

    REQUIRE(runtime.start().has_value());
    REQUIRE(wait_until([&] { return sleeper.entered >= 1; }));

    auto begin = clock_type::now();
    runtime.stop(5s);

    REQUIRE(clock_type::now() - begin < 2s);
}
