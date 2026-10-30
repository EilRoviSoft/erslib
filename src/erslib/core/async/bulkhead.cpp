#include "erslib/core/async/bulkhead.hpp"

#ifdef _HAS_BOOST_ASIO


// BulkheadState ---------------------------------------------------------------

// Member functions

ers::impl::BulkheadState::BulkheadState(size_t max_running, size_t max_waiting) :
    _max_running(max_running),
    _max_waiting(max_waiting) {
}


// Accessors

size_t ers::impl::BulkheadState::running() const {
    std::scoped_lock lock(_mutex);
    return _running;
}
size_t ers::impl::BulkheadState::waiting() const {
    std::scoped_lock lock(_mutex);
    return _waiting.size();
}


// Modifiers

ers::impl::BulkheadState::EAdmission ers::impl::BulkheadState::admit(launcher_t& launcher) {
    std::scoped_lock lock(_mutex);

    if (_running < _max_running) {
        ++_running;
        return EAdmission::Started;
    }

    if (_waiting.size() < _max_waiting) {
        _waiting.emplace_back(std::move(launcher));
        return EAdmission::Queued;
    }

    return EAdmission::Rejected;
}

void ers::impl::BulkheadState::finish() {
    launcher_t next;

    {
        std::scoped_lock lock(_mutex);

        if (_waiting.empty()) {
            --_running;
            return;
        }

        next = std::move(_waiting.front());
        _waiting.pop_front();
    }

    std::move(next)();
}

void ers::impl::BulkheadState::drop_waiting() {
    std::deque<launcher_t> dropped;

    std::scoped_lock lock(_mutex);
    dropped.swap(_waiting);
}


// Bulkhead --------------------------------------------------------------------

// Member functions

ers::impl::Bulkhead::Bulkhead(Scheduler& scheduler, options_t options) :
    _scheduler(&scheduler),
    _options(options),
    _state(std::make_shared<BulkheadState>(options.max_running, options.max_waiting)) {
}

ers::impl::Bulkhead::~Bulkhead() {
    clear();
}


// Accessors

ers::impl::Bulkhead::options_t ers::impl::Bulkhead::options() const noexcept {
    return _options;
}

size_t ers::impl::Bulkhead::running() const {
    return _state->running();
}
size_t ers::impl::Bulkhead::waiting() const {
    return _state->waiting();
}


// Modifiers

void ers::impl::Bulkhead::clear() {
    _state->drop_waiting();
}


#endif
