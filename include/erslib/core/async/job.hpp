#pragma once

#ifdef _HAS_BOOST_ASIO


// std
#include <chrono>
#include <future>
#include <utility>

// ers
#include <erslib/core/type/result.hpp>

#include "control.hpp"


// Implementation --------------------------------------------------------------

namespace ers::impl {
    template<typename T>
    class AsyncJob {
    public:
        // Member functions

        AsyncJob() = default;

        explicit AsyncJob(std::future<Result<T>> future, JobControlPtr control = {}) noexcept :
            _future(std::move(future)),
            _control(std::move(control)) {
        }


        // Observers

        bool valid() const noexcept { return _future.valid(); }
        bool rejected() const noexcept { return _rejected; }
        bool cancelled() const noexcept { return _control && _control->cancelled(); }

        bool ready() const {
            return _future.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
        }


        // Executors

        void wait() const {
            _future.wait();
        }

        template<typename Rep, typename Period>
        std::future_status wait_for(const std::chrono::duration<Rep, Period>& duration) const {
            return _future.wait_for(duration);
        }

        Result<T> get() { return _future.get(); }

        bool cancel() { return _control && _control->cancel(); }


        // Generators

        static AsyncJob rejected_with(Diagnostic reason) {
            std::promise<Result<T>> promise;
            promise.set_value(Result<T>(std::move(reason)));

            AsyncJob job(promise.get_future());
            job._rejected = true;

            return job;
        }


    private:
        std::future<Result<T>> _future;
        JobControlPtr _control;
        bool _rejected = false;
    };
}


// Exports ---------------------------------------------------------------------

namespace ers {
    using impl::AsyncJob;
}


#endif
