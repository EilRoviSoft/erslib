#include "erslib/core/async/process.hpp"

#if defined(_HAS_BOOST_ASIO) && defined(_HAS_BOOST_PROCESS)


// std
#include <algorithm>
#include <array>
#include <cerrno>
#include <csignal>
#include <mutex>
#include <string>
#include <variant>

// posix
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

// boost
#include <boost/process.hpp>
#include <boost/asio/as_tuple.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/asio/readable_pipe.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/this_coro.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/asio/writable_pipe.hpp>
#include <boost/asio/write.hpp>
#include <boost/asio/experimental/awaitable_operators.hpp>

// ers
#include <erslib/core/algorithm.hpp>


using namespace boost::asio::experimental::awaitable_operators;
namespace proc = boost::process::v2;


// Details ---------------------------------------------------------------------

namespace {
    struct new_group {
        template<typename Launcher>
        boost::system::error_code on_exec_setup(Launcher&, const proc::filesystem::path&, const char* const*&) const {
            if (::setpgid(0, 0) != 0)
                return { errno, boost::system::system_category() };
            return {};
        }
    };


    struct state_t {
        bool cancelled = false;
        bool timed_out = false;
        int exit_code = 0;
    };


    void ignore_sigpipe() {
        static std::once_flag once;

        std::call_once(once, [] {
            struct sigaction action {};
            action.sa_handler = SIG_IGN;
            sigemptyset(&action.sa_mask);
            sigaction(SIGPIPE, &action, nullptr);
        });
    }

    int shell_code(int status) {
        if (WIFEXITED(status))
            return WEXITSTATUS(status);

        if (WIFSIGNALED(status))
            return 128 + WTERMSIG(status);

        return status;
    }
}


namespace {
    boost::asio::awaitable<void> read_all(
        boost::asio::readable_pipe& pipe,
        std::string& target,
        size_t limit,
        boost::system::error_code& failure
    ) {
        co_await boost::asio::this_coro::throw_if_cancelled(false);

        std::array<char, 4096> buffer;

        while (true) {
            auto [error, count] = co_await pipe.async_read_some(
                boost::asio::buffer(buffer),
                boost::asio::as_tuple(boost::asio::use_awaitable)
            );

            if (count > 0 && target.size() < limit)
                target.append(buffer.data(), std::min(count, limit - target.size()));

            if (!error)
                continue;

            if (error != boost::asio::error::eof && error != boost::asio::error::operation_aborted)
                failure = error;

            co_return;
        }
    }

    boost::asio::awaitable<void> feed(boost::asio::writable_pipe& pipe, const std::string& input, bool enabled) {
        co_await boost::asio::this_coro::throw_if_cancelled(false);

        if (!enabled)
            co_return;


        co_await boost::asio::async_write(
            pipe,
            boost::asio::buffer(input),
            boost::asio::as_tuple(boost::asio::use_awaitable)
        );

        pipe.close();
    }

    boost::asio::awaitable<bool> watch(proc::process& child, int& code) {
        auto [error, _] = co_await child.async_wait(
            boost::asio::as_tuple(boost::asio::use_awaitable));

        if (!error)
            code = shell_code(child.native_exit_code());    

        co_return !error;
    }

    boost::asio::awaitable<bool> expire(std::chrono::milliseconds duration) {
        boost::asio::steady_timer timer(co_await boost::asio::this_coro::executor, duration);

        auto [error] = co_await timer.async_wait(
            boost::asio::as_tuple(boost::asio::use_awaitable));
        
        co_return !error;
    }

    boost::asio::awaitable<bool> exits_within(proc::process& child, int& code, std::chrono::milliseconds limit) {
        auto race = co_await(watch(child, code) || expire(limit));
        co_return race.index() == 0 && std::get<0>(race);
    }

    boost::asio::awaitable<void> supervise(proc::process& child, const ers::process_options_t& options, state_t& state) {
        co_await boost::asio::this_coro::throw_if_cancelled(false);

        pid_t pid = child.id();
        bool exited = false;

        if (options.timeout.count() > 0)
            exited = co_await exits_within(child, state.exit_code, options.timeout);
        else
            exited = co_await watch(child, state.exit_code);

        if (!exited) {
            auto cancellation = co_await boost::asio::this_coro::cancellation_state;

            state.cancelled = cancellation.cancelled() != boost::asio::cancellation_type::none;
            state.timed_out = !state.cancelled;

            ::kill(-pid, SIGTERM);

            if (!co_await exits_within(child, state.exit_code, options.grace)) {
                ::kill(-pid, SIGKILL);
                co_await watch(child, state.exit_code);
            }
        }

        ::kill(-pid, SIGKILL);
    }
}


// Implementation --------------------------------------------------------------

ers::impl::AsyncTask<ers::process_output_t> ers::impl::run_process(std::vector<std::string> args, process_options_t options) {
    if (args.empty())
        co_return make_error("The process command is empty.");

    ignore_sigpipe();


    auto executor = co_await boost::asio::this_coro::executor;
    bool has_input = !options.input.empty();


    auto executable = proc::environment::find_executable(args[0]);

    if (executable.empty()) {
        co_return make_error("Could not start '{}': executable not found.",
            args[0]);
    }


    boost::asio::readable_pipe out(executor);
    boost::asio::readable_pipe err(executor);
    boost::asio::writable_pipe in(executor);

    std::vector rest(args.begin() + 1, args.end());
    proc::default_process_launcher launcher;
    boost::system::error_code error;

    auto child = has_input
        ? launcher(
            executor,
            error,
            executable,
            rest,
            proc::process_stdio { .in = in, .out = out, .err = err },
            new_group {}
        )
        : launcher(
            executor,
            error,
            executable,
            rest,
            proc::process_stdio { .in = nullptr, .out = out, .err = err },
            new_group {}
        );

    if (error) {
        co_return make_error("Could not start '{}': {}",
            args[0], error.message());
    }


    process_output_t output;
    state_t state;

    boost::system::error_code out_failure;
    boost::system::error_code err_failure;

    co_await (
        read_all(out, output.out, options.max_output, out_failure)
        && read_all(err, output.err, options.max_output, err_failure)
        && feed(in, options.input, has_input)
        && supervise(child, options, state)
    );

    if (out_failure || err_failure) {
        co_return make_error("Could not read the process output: {}",
            (out_failure ? out_failure : err_failure).message());
    }


    if (state.cancelled)
        co_return make_error("The process was cancelled.");

    if (state.timed_out)
        co_return make_error("The process timed out.");


    output.exit_code = state.exit_code;
    co_return output;
}

ers::impl::AsyncTask<ers::process_output_t> ers::impl::run_command(std::string line, process_options_t options) {
    auto args = split_args(line);

    if (!args)
        co_return std::move(args).extract_error();

    co_return co_await run_process(std::move(*args), std::move(options));
}


#endif
