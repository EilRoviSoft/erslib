#pragma once

#ifdef _HAS_BOOST_ASIO

// std
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstddef>
#include <exception>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <utility>

// boost
#include <boost/asio/cancellation_signal.hpp>
#include <boost/asio/error.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/thread_pool.hpp>
#include <boost/system/system_error.hpp>

// ers
#include <erslib/core/async/task.hpp>
#include <erslib/core/type/maybe.hpp>
#include <erslib/core/type/result.hpp>

// export
#include <erslib/export.hpp>


// Fwd -------------------------------------------------------------------------

namespace ers::impl {
    class JobTracker;

    using JobTrackerPtr = std::shared_ptr<JobTracker>;


    class JobControl;

    using JobControlPtr = std::shared_ptr<JobControl>;
    using JobControlWeak = std::weak_ptr<JobControl>;
}


// Details ---------------------------------------------------------------------

namespace ers::impl {
    struct spawn_options_t {
        bool interruptible = false;
    };


    class ERSLIB_EXPORT JobTracker {
    public:
        // Accessors

        size_t active() const;


        // Modifiers

        void enter(const JobControlPtr& control);
        void leave(const JobControl* control);

        size_t cancel_all();


        // Executors

        bool wait_idle(std::chrono::milliseconds grace);


    private:
        mutable std::mutex _mutex;
        std::condition_variable _cv;
        std::unordered_map<const JobControl*, JobControlWeak> _jobs;
    };
}


// Implementation --------------------------------------------------------------

namespace ers::impl {
    class ERSLIB_EXPORT JobControl : public std::enable_shared_from_this<JobControl> {
    public:
        // Member types

        using strand_t = boost::asio::strand<boost::asio::thread_pool::executor_type>;


        // Member functions

        JobControl(bool interruptible, JobTrackerPtr tracker);
        ~JobControl();

        JobControl(const JobControl&) = delete;
        JobControl& operator=(const JobControl&) = delete;


        // Accessors

        boost::asio::cancellation_slot slot() noexcept;


        // Observers

        bool interruptible() const noexcept;
        bool cancelled() const noexcept;


        // Modifiers

        void attach(strand_t strand);
        bool cancel();
        void finish() noexcept;


    private:
        mutable std::mutex _mutex;
        maybe<strand_t> _strand;
        bool _interruptible;
        std::atomic_bool _cancelled = false;
        std::atomic_bool _finished = false;
        JobTrackerPtr _tracker;
        boost::asio::cancellation_signal _signal;
    };
}


namespace ers::impl {
    template<typename T>
    AsyncTask<T> cancellable(std::shared_ptr<JobControl> control, AsyncTask<T> task) {
        if (control->cancelled())
            co_return make_error("The job was cancelled.");

        Result<T> result;
        std::exception_ptr error;
        bool aborted = false;

        try {
            result = co_await std::move(task);
        } catch (const boost::system::system_error& e) {
            if (e.code() == boost::asio::error::operation_aborted)
                aborted = true;
            else
                error = std::current_exception();
        } catch (...) {
            error = std::current_exception();
        }

        if (aborted)
            co_return make_error("The job was cancelled.");

        if (error)
            std::rethrow_exception(error);

        co_return result;
    }
}


#endif
