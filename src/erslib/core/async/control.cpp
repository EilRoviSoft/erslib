#include "erslib/core/async/control.hpp"

#ifdef _HAS_BOOST_ASIO


// std
#include <vector>

// boost
#include <ranges>
#include <boost/asio/post.hpp>


// JobTracker ------------------------------------------------------------------

// Accessors

size_t ers::impl::JobTracker::active() const {
    std::scoped_lock lock(_mutex);
    return _jobs.size();
}


// Modifiers

void ers::impl::JobTracker::enter(const JobControlPtr& control) {
    std::scoped_lock lock(_mutex);
    _jobs.try_emplace(control.get(), control);
}

void ers::impl::JobTracker::leave(const JobControl* control) {
    {
        std::scoped_lock lock(_mutex);
        _jobs.erase(control);
    }

    _cv.notify_all();
}

size_t ers::impl::JobTracker::cancel_all() {
    std::vector<JobControlPtr> jobs;

    {
        std::scoped_lock lock(_mutex);

        for (auto& weak : _jobs | std::views::values) {
            if (auto job = weak.lock())
                jobs.push_back(std::move(job));
        }
    }


    size_t cancelled = 0;

    for (auto& job : jobs) {
        if (job->cancel())
            ++cancelled;
    }


    return cancelled;
}


// Executors

bool ers::impl::JobTracker::wait_idle(std::chrono::milliseconds grace) {
    std::unique_lock lock(_mutex);
    return _cv.wait_for(lock, grace, [this] { return _jobs.empty(); });
}


// JobControl ------------------------------------------------------------------

// Member functions

ers::impl::JobControl::JobControl(bool interruptible, JobTrackerPtr tracker) :
    _interruptible(interruptible),
    _tracker(std::move(tracker)) {
}

ers::impl::JobControl::~JobControl() {
    finish();
}


// Accessors

boost::asio::cancellation_slot ers::impl::JobControl::slot() noexcept {
    return _signal.slot();
}


// Observers

bool ers::impl::JobControl::interruptible() const noexcept {
    return _interruptible;
}

bool ers::impl::JobControl::cancelled() const noexcept {
    return _cancelled;
}


// Modifiers

void ers::impl::JobControl::attach(strand_t strand) {
    std::scoped_lock lock(_mutex);
    _strand.emplace(std::move(strand));
}

bool ers::impl::JobControl::cancel() {
    if (!_interruptible)
        return false;

    std::scoped_lock lock(_mutex);

    if (_finished)
        return false;

    if (_cancelled.exchange(true))
        return true;

    if (_strand) {
        boost::asio::post(*_strand, [self = shared_from_this()] {
            self->_signal.emit(boost::asio::cancellation_type::terminal);
        });
    }

    return true;
}

void ers::impl::JobControl::finish() noexcept {
    {
        std::scoped_lock lock(_mutex);

        if (_finished.exchange(true))
            return;

        _strand.reset();
    }

    if (_tracker)
        _tracker->leave(this);
}


#endif
