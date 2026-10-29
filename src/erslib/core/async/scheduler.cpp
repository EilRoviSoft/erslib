#include "erslib/core/async/scheduler.hpp"

#ifdef _HAS_BOOST_ASIO


// Member functions

ers::Scheduler::Scheduler()
    : Scheduler(options_t {}) {
}

ers::impl::Scheduler::Scheduler(options_t options) :
    _options(options),
    _pool(_options.threads),
    _blocking_pool(_options.blocking_threads) {
}


// Accessors

size_t ers::Scheduler::options_t::capacity() const noexcept {
    return threads + blocking_threads;
}


bool ers::Scheduler::stopped() const noexcept {
    return _stopped;
}

ers::Scheduler::options_t ers::Scheduler::options() const noexcept {
    return _options;
}

boost::asio::thread_pool::executor_type ers::Scheduler::executor() noexcept {
    return _pool.get_executor();
}


// Modifiers

void ers::impl::Scheduler::stop() {
    if (_stopped.exchange(true))
        return;


    _blocking_pool.stop();
    _blocking_pool.join();

    _pool.stop();
    _pool.join();
}


#endif
