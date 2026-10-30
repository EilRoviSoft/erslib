#include "erslib/core/async/runtime.hpp"

#ifdef _HAS_BOOST_ASIO


// std
#include <exception>

// boost
#include <boost/asio/error.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/this_coro.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/system/system_error.hpp>

#include "erslib/core/async/scheduler.hpp"


// Details ---------------------------------------------------------------------

namespace {
    ers::AsyncTask<void> periodic_loop(
        std::chrono::milliseconds interval,
        ers::impl::tick_t tick,
        ers::impl::periodic_options_t options
    ) {
        auto executor = co_await boost::asio::this_coro::executor;
        bool first = options.immediate;

        while (true) {
            if (!first) {
                boost::asio::steady_timer timer(executor, interval);
                co_await timer.async_wait(boost::asio::use_awaitable);
            }

            first = false;


            ers::Status result;

            try {
                result = co_await tick();
            } catch (const boost::system::system_error& e) {
                if (e.code() == boost::asio::error::operation_aborted)
                    throw;

                result = ers::make_error("The periodic task '{}' failed: {}",
                    options.name, e.what());
            } catch (const std::exception& e) {
                result = ers::make_error("The periodic task '{}' failed: {}",
                    options.name, e.what());
            }

            if (!result && options.on_error)
                options.on_error(result.error());
        }
    }
}


// AsyncRegistry ---------------------------------------------------------------

// Member functions

ers::impl::AsyncRegistry::AsyncRegistry(AsyncRuntime& runtime) noexcept :
    _runtime(&runtime) {
}


// Accessors

ers::Scheduler& ers::impl::AsyncRegistry::scheduler() noexcept {
    return _runtime->_scheduler;
}


// Generators

ers::Bulkhead& ers::impl::AsyncRegistry::bulkhead(Bulkhead::options_t options) {
    return *_runtime->_bulkheads.emplace_back(std::make_unique<Bulkhead>(_runtime->_scheduler, options));
}

void ers::impl::AsyncRegistry::every(std::chrono::milliseconds interval, tick_t tick, periodic_options_t options) {
    _runtime->_pending.push_back({
        .interval = interval,
        .tick     = std::move(tick),
        .options  = std::move(options)
    });
}


// AsyncRuntime ----------------------------------------------------------------

// Member functions

ers::impl::AsyncRuntime::AsyncRuntime() :
    AsyncRuntime(Scheduler::options_t { .threads = 1, .blocking_threads = 1 }) {
}
ers::impl::AsyncRuntime::AsyncRuntime(Scheduler::options_t options) :
    _scheduler(options) {
}

ers::impl::AsyncRuntime::~AsyncRuntime() {
    stop();
}

// Accessors

ers::Scheduler& ers::impl::AsyncRuntime::scheduler() noexcept {
    return _scheduler;
}


// Modifiers

ers::Status ers::impl::AsyncRuntime::start() {
    if (_state != EState::Idle)
        return make_error("The runtime was already started.");


    AsyncRegistry registry(*this);

    for (auto* worker : _workers)
        worker->declare(registry);

    for (size_t i = 0; i < _services.size(); ++i) {
        auto result = _services[i]->start();

        if (result)
            continue;


        auto error = make_error("The service '{}' failed to start: {}",
            _services[i]->name(), result.error().message());

        while (i-- > 0)
            _services[i]->stop();


        _scheduler.stop();
        _state = EState::Stopped;

        return error;
    }

    for (auto& entry : _pending) {
        _periodic.push_back(_scheduler.spawn(
            periodic_loop(entry.interval, entry.tick, entry.options),
            { .interruptible = true }
        ));
    }


    _state = EState::Started;

    return ok;
}

void ers::impl::AsyncRuntime::stop(std::chrono::milliseconds grace) {
    if (_state == EState::Stopped)
        return;


    bool started = _state == EState::Started;
    _state = EState::Stopped;

    for (auto& job : _periodic)
        job.cancel();

    for (auto& job : _periodic)
        job.wait_for(grace);

    if (started) {
        for (size_t i = _services.size(); i-- > 0;)
            _services[i]->stop();
    }

    _scheduler.stop(grace);

    for (auto& bulkhead : _bulkheads)
        bulkhead->clear();
}


#endif
