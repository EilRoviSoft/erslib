#pragma once

#ifdef _HAS_BOOST_ASIO


// std
#include <chrono>
#include <functional>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

// ers
#include <erslib/core/async/bulkhead.hpp>
#include <erslib/core/async/job.hpp>
#include <erslib/core/async/scheduler.hpp>
#include <erslib/core/async/service.hpp>
#include <erslib/core/async/task.hpp>
#include <erslib/core/async/worker.hpp>
#include <erslib/core/type/result.hpp>

// export
#include <erslib/export.hpp>


// Fwd -------------------------------------------------------------------------

namespace ers::impl {
    class AsyncRuntime;
}


// Details ---------------------------------------------------------------------

namespace ers::impl {
    using tick_t = std::function<AsyncTask<void>()>;


    struct periodic_options_t {
        std::string name;
        bool immediate = false;
        std::function<void(const Diagnostic&)> on_error;
    };
}


// Implementation --------------------------------------------------------------

namespace ers::impl {
    class ERSLIB_EXPORT AsyncRegistry {
    public:
        // Member functions

        explicit AsyncRegistry(AsyncRuntime& runtime) noexcept;


        // Accessors

        Scheduler& scheduler() noexcept;


        // Generators

        Bulkhead& bulkhead(Bulkhead::options_t options);

        void every(std::chrono::milliseconds interval, tick_t tick, periodic_options_t options = {});


    private:
        AsyncRuntime* _runtime;
    };
}

namespace ers::impl {
    class ERSLIB_EXPORT AsyncRuntime {
        friend class AsyncRegistry;


        enum class EState {
            Idle,
            Started,
            Stopped,
        };

        struct periodic_t {
            std::chrono::milliseconds interval;
            tick_t tick;
            periodic_options_t options;
        };


    public:
        // Member functions

        AsyncRuntime();
        explicit AsyncRuntime(Scheduler::options_t options);

        AsyncRuntime(const AsyncRuntime&) = delete;
        AsyncRuntime& operator=(const AsyncRuntime&) = delete;

        ~AsyncRuntime();


        // Accessors

        Scheduler& scheduler() noexcept;


        // Modifiers

        template<typename T, typename... Args>
        T& add(Args&&... args) {
            auto owned = std::make_unique<T>(std::forward<Args>(args)...);
            T& service = *owned;

            if constexpr (std::is_base_of_v<IWorker, T>)
                _workers.push_back(static_cast<IWorker*>(&service));

            _services.push_back(std::move(owned));
            return service;
        }

        Status start();
        void stop(std::chrono::milliseconds grace = std::chrono::seconds(5));


    private:
        Scheduler _scheduler;
        std::vector<std::unique_ptr<Bulkhead>> _bulkheads;
        std::vector<periodic_t> _pending;
        std::vector<AsyncJob<void>> _periodic;
        std::vector<IWorker*> _workers;
        std::vector<std::unique_ptr<IService>> _services;
        EState _state = EState::Idle;
    };
}


// Exports ---------------------------------------------------------------------

namespace ers {
    using impl::periodic_options_t;

    using impl::AsyncRegistry;
    using impl::AsyncRuntime;
}


#endif
