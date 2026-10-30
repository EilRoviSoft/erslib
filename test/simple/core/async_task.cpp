// catch2
#include <catch2/catch_test_macros.hpp>

// std
#include <stdexcept>
#include <string_view>

// boost
#include <boost/asio.hpp>

// ers
#include <erslib/core/async/task.hpp>

// test
#include "support/test_case.hpp"


namespace {
    int destroyed = 0;

    struct Guard {
        ~Guard() { ++destroyed; }
    };


    ers::AsyncTask<int> add(int a, int b) {
        co_return a + b;
    }

    ers::AsyncTask<int> chain() {
        ERS_CO_TRY_AS(int x, co_await add(1, 2));
        ERS_CO_TRY_AS(int y, co_await add(x, 3));
        co_return y * 2;
    }

    ers::AsyncTask<void> fail() {
        co_return ers::make_error("boom {}", 7);
    }

    ers::AsyncTask<int> propagates() {
        Guard guard;
        ERS_CO_TRY(co_await fail());
        co_return 1;
    }

    ers::AsyncTask<int> throws() {
        throw std::runtime_error("exception");
        co_return 0;
    }

    ers::AsyncTask<void> nothing() {
        co_return ers::ok;
    }

    ers::AsyncTask<int> posted() {
        co_await boost::asio::post(co_await boost::asio::this_coro::executor, boost::asio::use_awaitable);
        co_return 5;
    }
}


ERS_TEST_CASE("chain", "[async]") {
    REQUIRE(*ers::sync_wait(add(2, 3)) == 5);
    REQUIRE(*ers::sync_wait(chain()) == 12);
}

ERS_TEST_CASE("error", "[async]") {
    auto result = ers::sync_wait(fail());

    REQUIRE(!result);
    REQUIRE(result.error().message().find("boom 7") != std::string_view::npos);
}

ERS_TEST_CASE("locals", "[async]") {
    destroyed = 0;

    auto result = ers::sync_wait(propagates());

    REQUIRE(!result);
    REQUIRE(destroyed == 1);
}

ERS_TEST_CASE("exception", "[async]") {
    auto result = ers::sync_wait(throws());

    REQUIRE(!result);
    REQUIRE(result.error().message().find("exception") != std::string_view::npos);
}

ERS_TEST_CASE("void", "[async]") {
    REQUIRE(ers::sync_wait(nothing()).has_value());
}

ERS_TEST_CASE("post", "[async]") {
    REQUIRE(*ers::sync_wait(posted()) == 5);
}
