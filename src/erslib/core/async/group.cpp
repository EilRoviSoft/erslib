#include "erslib/core/async/group.hpp"

#ifdef _HAS_BOOST_ASIO


// GroupState ------------------------------------------------------------------

// Member functions

ers::impl::GroupState::GroupState(size_t max_running, size_t max_waiting) :
    _max_running(max_running),
    _max_waiting(max_waiting) {
}


// Accessors

size_t ers::impl::GroupState::running() const {
    std::scoped_lock lock(_mutex);
    return _running;
}
size_t ers::impl::GroupState::waiting() const {
    std::scoped_lock lock(_mutex);
    return _waiting.size();
}


// Modifiers

ers::impl::GroupState::EAdmission ers::impl::GroupState::admit(launcher_t& launcher) {
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

void ers::impl::GroupState::finish() {
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

    next();
}

void ers::impl::GroupState::drop_waiting() {
    std::deque<launcher_t> dropped;

    std::scoped_lock lock(_mutex);
    dropped.swap(_waiting);
}


// Group -----------------------------------------------------------------------

// Member functions

ers::Group::Group(Scheduler& scheduler, options_t options) :
    _scheduler(&scheduler),
    _options(options),
    _state(std::make_shared<GroupState>(options.max_running, options.max_waiting)) {
}

ers::Group::~Group() {
    clear();
}


// Accessors

ers::Group::options_t ers::Group::options() const noexcept {
    return _options;
}

size_t ers::Group::running() const {
    return _state->running();
}
size_t ers::Group::waiting() const {
    return _state->waiting();
}


// Modifiers

void ers::Group::clear() {
    _state->drop_waiting();
}


#endif
