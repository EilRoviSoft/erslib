#include "erslib/core/async/task.hpp"

// boost
#include <boost/asio/post.hpp>
#include <boost/asio/this_coro.hpp>
#include <boost/asio/use_awaitable.hpp>


boost::asio::awaitable<void> ers::impl::yield() {
    auto executor = co_await boost::asio::this_coro::executor;
    co_await boost::asio::post(executor, boost::asio::use_awaitable);
}
