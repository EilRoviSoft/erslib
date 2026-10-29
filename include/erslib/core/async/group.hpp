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
    class GroupState {
    public:
        // Member types

        using launcher_t = std::move_only_function<void()>;

        enum class EAdmission {
            Started,
            Queued,
            Rejected,
        };


        // Member functions

        GroupState(size_t max_running, size_t max_waiting);


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
    Task<T> guarded(std::shared_ptr<GroupState> state, Task<T> task) {
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

    template<typename T>
    GroupState::launcher_t make_launcher(
        boost::asio::thread_pool::executor_type executor,
        Task<T> task,
        Completion<T> completion
    ) {
        return [executor, task = std::move(task), completion = std::move(completion)]() mutable {
            boost::asio::co_spawn(executor, std::move(task), std::move(completion));
        };
    }
}


// Implementation --------------------------------------------------------------

namespace ers::impl {
    class Group {
    public:
        // Member types

        struct options_t {
            size_t max_running = 1;
            size_t max_waiting = 0;
        };


        // Member functions

        Group(Scheduler& scheduler, options_t options);

        Group(const Group&) = delete;
        Group& operator=(const Group&) = delete;

        ~Group();


        // Accessors

        options_t options() const noexcept;


        // Observers

        size_t running() const;
        size_t waiting() const;


        // Modifiers

        void clear();


        // Generators

        template<typename T>
        Job<T> spawn(Task<T> task) {
            if (_scheduler->stopped())
                return Job<T>::rejected_with(make_error("The scheduler is stopped"));

            std::promise<Result<T>> promise;
            auto future = promise.get_future();

            auto launcher = make_launcher<T>(
                _scheduler->executor(),
                guarded<T>(_state, std::move(task)),
                Completion<T>(std::move(promise))
            );

            switch (_state->admit(launcher)) {
                case GroupState::EAdmission::Started:
                    launcher();
                    break;

                case GroupState::EAdmission::Queued:
                    break;

                case GroupState::EAdmission::Rejected:
                    return Job<T>::rejected_with(make_error("The group is full"));
            }

            return Job<T>(std::move(future));
        }


    private:
        Scheduler* _scheduler;
        const options_t _options;
        std::shared_ptr<GroupState> _state;
    };
}


// Exports ---------------------------------------------------------------------

namespace ers {
    using impl::Group;
}


#endif
