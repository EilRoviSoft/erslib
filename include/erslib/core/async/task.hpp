#pragma once

#ifdef _HAS_BOOST_ASIO


// std
#include <exception>
#include <utility>

// boost
#include <boost/asio/awaitable.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/io_context.hpp>

// ers
#include <erslib/core/type/result.hpp>


// Implementation --------------------------------------------------------------

namespace ers::impl {
    template<typename T>
    using AsyncTask = boost::asio::awaitable<Result<T>>;


    template<typename T>
    Result<T> to_result(std::exception_ptr error, Result<T> value) {
        if (!error)
            return value;

        try {
            std::rethrow_exception(error);
        } catch (const std::exception& e) {
            return make_error("AsyncTask threw an exception: {}", e.what());
        } catch (...) {
            return make_error("AsyncTask threw an unknown exception.");
        }
    }


    template<typename T>
    Result<T> sync_wait(AsyncTask<T> task) {
        boost::asio::io_context io;
        Result<T> out = make_error("sync_wait did not finish");

        boost::asio::co_spawn(io, std::move(task), [&](std::exception_ptr error, Result<T> value) {
            out = to_result<T>(error, std::move(value));
        });

        io.run();
        return out;
    }


    boost::asio::awaitable<void> yield();
    AsyncTask<void> checkpoint();
}


// Macros ----------------------------------------------------------------------

#define ERS_CO_TRY(...) \
    do { \
        auto&& _ers_co_try = (__VA_ARGS__); \
        if (!_ers_co_try) \
            co_return std::move(_ers_co_try).extract_error(); \
    } while (false)

#define ERS_CO_TRY_AS(DECLARATION, ...) \
    auto&& ERS_CO_NAME(_ers_co_try_, __LINE__) = (__VA_ARGS__); \
    if (!ERS_CO_NAME(_ers_co_try_, __LINE__)) \
        co_return std::move(ERS_CO_NAME(_ers_co_try_, __LINE__)).extract_error(); \
    DECLARATION = ers::impl::unwrap(std::move(ERS_CO_NAME(_ers_co_try_, __LINE__)))

#define ERS_CO_NAME(PREFIX, LINE) ERS_CO_JOIN(PREFIX, LINE)
#define ERS_CO_JOIN(PREFIX, LINE) PREFIX##LINE


// Exports ---------------------------------------------------------------------

namespace ers {
    using impl::AsyncTask;

    using impl::to_result;
    using impl::sync_wait;

    using impl::yield;
    using impl::checkpoint;
}


#endif
