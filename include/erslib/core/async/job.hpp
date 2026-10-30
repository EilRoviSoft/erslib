#pragma once

#ifdef _HAS_BOOST_ASIO


// std
#include <chrono>
#include <future>
#include <utility>

// ers
#include <erslib/core/type/result.hpp>


// Implementation --------------------------------------------------------------

namespace ers::impl {
    template<typename T>
    class AsyncJob {
    public:
        // Member functions

        AsyncJob() = default;

        explicit AsyncJob(std::future<Result<T>> future) noexcept :
            _future(std::move(future)) {
        }


        // Observers

        bool valid() const noexcept { return _future.valid(); }
        bool rejected() const noexcept { return _rejected; }

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
        bool _rejected = false;
    };
}


// Exports ---------------------------------------------------------------------

namespace ers {
    using impl::AsyncJob;
}


#endif
