#pragma once

#ifdef _HAS_BOOST_ASIO


// std
#include <atomic>
#include <chrono>
#include <future>
#include <type_traits>
#include <utility>

// boost
#include <boost/asio/as_tuple.hpp>
#include <boost/asio/bind_cancellation_slot.hpp>
#include <boost/asio/post.hpp>
#include <boost/asio/thread_pool.hpp>
#include <boost/asio/use_awaitable.hpp>

// ers
#include <erslib/core/async/control.hpp>
#include <erslib/core/async/job.hpp>
#include <erslib/core/async/task.hpp>
#include <erslib/core/type/result.hpp>

// export
#include <erslib/export.hpp>


// Details ---------------------------------------------------------------------

namespace ers::impl {
    template<typename T>
    class Completion {
    public:
        explicit Completion(std::promise<Result<T>> promise, JobControlPtr control) :
            _promise(std::move(promise)),
            _control(std::move(control)) {
        }

        Completion(Completion&& other) noexcept :
            _promise(std::move(other._promise)),
            _control(std::move(other._control)),
            _pending(std::exchange(other._pending, false)) {
        }

        ~Completion() {
            if (_pending)
                _promise.set_value(make_error("The job was dropped before it finished."));

            if (_control)
                _control->finish();
        }


        void operator()(std::exception_ptr error, Result<T> value) {
            _pending = false;
            _promise.set_value(to_result<T>(error, std::move(value)));
        }


    private:
        std::promise<Result<T>> _promise;
        JobControlPtr _control;
        bool _pending = true;
    };


    template<typename T>
    void launch(
        const JobControlPtr& control,
        boost::asio::thread_pool::executor_type executor,
        AsyncTask<T> task,
        Completion<T> completion
    ) {
        auto strand = boost::asio::make_strand(executor);
        control->attach(strand);

        boost::asio::co_spawn(
            strand,
            std::move(task),
            boost::asio::bind_cancellation_slot(control->slot(), std::move(completion))
        );
    }
}


// Implementation --------------------------------------------------------------

namespace ers::impl {
    class ERSLIB_EXPORT Scheduler {
    public:
        // Member types

        struct options_t {
            size_t threads = 0;
            size_t blocking_threads = 0;


            size_t capacity() const noexcept;
        };


        // Member functions

        Scheduler();
        explicit Scheduler(options_t options);

        Scheduler(const Scheduler&) = delete;
        Scheduler& operator=(const Scheduler&) = delete;


        // Accessors

        bool stopped() const noexcept;

        options_t options() const noexcept;

        boost::asio::thread_pool::executor_type executor() noexcept;


        // Modifiers

        void stop(std::chrono::milliseconds grace = std::chrono::milliseconds::zero());


        // Generators

        template<typename T>
        AsyncJob<T> spawn(AsyncTask<T> task, spawn_options_t spawn_options = {}) {
            if (_stopped)
                return AsyncJob<T>::rejected_with(make_error("The scheduler is stopped."));


            std::promise<Result<T>> promise;
            auto future = promise.get_future();
            auto control = make_control(spawn_options);

            launch<T>(
                control,
                _pool.get_executor(),
                cancellable<T>(control, std::move(task)),
                Completion<T>(std::move(promise), control)
            );


            return AsyncJob<T>(std::move(future), std::move(control));
        }


        template<typename F>
        auto blocking(F fn) -> AsyncTask<typename std::invoke_result_t<F&>::value_type> {
            using value_type = typename std::invoke_result_t<F&>::value_type;

            auto [error, result] = co_await boost::asio::co_spawn(
                _blocking_pool,
                [](F fn) -> AsyncTask<value_type> { co_return fn(); }(std::move(fn)),
                boost::asio::as_tuple(boost::asio::use_awaitable)
            );

            co_return to_result<value_type>(error, std::move(result));
        }


        JobControlPtr make_control(spawn_options_t spawn_options);


    private:
        const options_t _options;
        std::atomic_bool _stopped = false;
        JobTrackerPtr _tracker;
        boost::asio::thread_pool _pool;
        boost::asio::thread_pool _blocking_pool;
    };
}


// Exports ---------------------------------------------------------------------

namespace ers {
    using impl::spawn_options_t;

    using impl::Scheduler;
}


#endif
