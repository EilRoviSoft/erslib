#pragma once

#ifdef _HAS_BOOST_ASIO

// std
#include <cstddef>
#include <deque>
#include <exception>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <utility>

// ers
#include <erslib/core/async/job.hpp>
#include <erslib/core/async/scheduler.hpp>


// Details ---------------------------------------------------------------------

namespace ers::impl {
    class ERSLIB_EXPORT BulkheadState {
    public:
        // Member types

        using launcher_t = std::move_only_function<void() &&>;

        enum class EAdmission {
            Started,
            Queued,
            Rejected,
        };


        // Member functions

        BulkheadState(size_t max_running, size_t max_waiting);


        // Observers

        size_t running() const;
        size_t waiting() const;


        // Modifiers

        EAdmission admit(launcher_t& launcher);

        void finish();

        void drop_waiting();


    private:
        mutable std::mutex _mutex;
        std::deque<launcher_t> _waiting;
        size_t _running = 0;
        size_t _max_running;
        size_t _max_waiting;
    };


    template<typename T>
    AsyncTask<T> guarded(std::shared_ptr<BulkheadState> state, AsyncTask<T> task) {
        Result<T> result;
        std::exception_ptr error;

        try {
            result = co_await std::move(task);
        } catch (...) {
            error = std::current_exception();
        }

        state->finish();

        if (error)
            std::rethrow_exception(error);

        co_return result;
    }
}


// Implementation --------------------------------------------------------------

namespace ers::impl {
    class ERSLIB_EXPORT Bulkhead {
    public:
        // Member types

        struct options_t {
            size_t max_running = 1;
            size_t max_waiting = 0;
        };


        // Member functions

        Bulkhead(Scheduler& scheduler, options_t options);

        Bulkhead(const Bulkhead&) = delete;
        Bulkhead& operator=(const Bulkhead&) = delete;

        ~Bulkhead();


        // Accessors

        options_t options() const noexcept;


        // Observers

        size_t running() const;
        size_t waiting() const;


        // Modifiers

        void clear();


        // Generators

        template<typename T>
        AsyncJob<T> spawn(AsyncTask<T> task, spawn_options_t spawn_options = {}) {
            if (_scheduler->stopped())
                return AsyncJob<T>::rejected_with(make_error("The scheduler is stopped."));


            std::promise<Result<T>> promise;
            auto future = promise.get_future();
            auto control = _scheduler->make_control(spawn_options);

            BulkheadState::launcher_t launcher = std::bind_front(
                &launch<T>,
                control,
                _scheduler->executor(),
                guarded<T>(_state, cancellable<T>(control, std::move(task))),
                Completion<T>(std::move(promise), control)
            );


            switch (_state->admit(launcher)) {
                case BulkheadState::EAdmission::Started:
                    std::move(launcher)();
                    break;

                case BulkheadState::EAdmission::Queued:
                    break;

                case BulkheadState::EAdmission::Rejected:
                    return AsyncJob<T>::rejected_with(make_error("The bulkhead is full."));
            }


            return AsyncJob<T>(std::move(future), std::move(control));
        }


    private:
        Scheduler* _scheduler;
        const options_t _options;
        std::shared_ptr<BulkheadState> _state;
    };
}


// Exports ---------------------------------------------------------------------

namespace ers {
    using impl::Bulkhead;
}


#endif
