// catch2
#include <catch2/catch_test_macros.hpp>

// std
#include <chrono>
#include <csignal>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <string>
#include <string_view>
#include <thread>

// posix
#include <unistd.h>

// ers
#include <erslib/core/algorithm.hpp>
#include <erslib/core/filesystem.hpp>
#include <erslib/core/async/process.hpp>
#include <erslib/core/async/scheduler.hpp>

// test
#include "support/temp_tree.hpp"
#include "support/test_case.hpp"


// Details ---------------------------------------------------------------------

namespace {
    using namespace std::chrono_literals;

    using clock_type = std::chrono::steady_clock;


    bool mentions(const ers::Result<ers::process_output_t>& result, std::string_view text) {
        return !result && result.error().message().find(text) != std::string_view::npos;
    }

    fs::path scratch(std::string_view name) {
        return fs::temp_directory_path() / ("ers_process_" + std::to_string(::getpid()) + "_" + std::string(name));
    }

    pid_t read_pid(const fs::path& path) {
        std::ifstream file(path);
        pid_t pid = 0;
        file >> pid;

        return pid;
    }

    bool gone(pid_t pid) {
        return pid > 0 && ::kill(pid, 0) == -1 && errno == ESRCH;
    }

    ers::AsyncTask<ers::process_output_t> shell(std::string script, ers::process_options_t options = {}) {
        return ers::run_command(std::format("sh -c {}", ers::quote_arg(script)),
            std::move(options));
    }

    bool wait_for_file(const fs::path& path, std::chrono::milliseconds limit = 5s) {
        auto deadline = clock_type::now() + limit;

        while (!fs::exists(path) || fs::file_size(path) == 0) {
            if (clock_type::now() > deadline)
                return false;

            std::this_thread::sleep_for(5ms);
        }

        return true;
    }
}


// Output ----------------------------------------------------------------------

ERS_TEST_CASE("echo", "[async]") {
    auto result = ers::sync_wait(shell("echo hello"));

    REQUIRE(result.has_value());
    REQUIRE(result->exit_code == 0);
    REQUIRE(result->out == "hello\n");
    REQUIRE(result->err.empty());
}

ERS_TEST_CASE("status", "[async]") {
    auto result = ers::sync_wait(shell("echo oops >&2; exit 3"));

    REQUIRE(result.has_value());
    REQUIRE(result->exit_code == 3);
    REQUIRE(result->out.empty());
    REQUIRE(result->err == "oops\n");
}

ERS_TEST_CASE("signal", "[async]") {
    auto result = ers::sync_wait(shell("kill -9 $$"));

    REQUIRE(result.has_value());
    REQUIRE(result->exit_code == 128 + 9);
}

ERS_TEST_CASE("missing", "[async]") {
    auto result = ers::sync_wait(ers::run_command("/nonexistent/ers-binary"));

    REQUIRE(mentions(result, "Could not start"));
}

ERS_TEST_CASE("empty", "[async]") {
    REQUIRE(!ers::sync_wait(ers::run_process({})));
}

ERS_TEST_CASE("cap", "[async]") {
    auto result = ers::sync_wait(shell("head -c 200000 /dev/zero",
        { .max_output = 1000 }));

    REQUIRE(result.has_value());
    REQUIRE(result->exit_code == 0);
    REQUIRE(result->out.size() == 1000);
}

ERS_TEST_CASE("nine", "[async]") {
    auto result = ers::sync_wait(shell("exit 9"));

    REQUIRE(result.has_value());
    REQUIRE(result->exit_code == 9);
}


// Input -----------------------------------------------------------------------

ERS_TEST_CASE("input", "[async]") {
    auto result = ers::sync_wait(ers::run_command("cat",
        { .input = "ping" }));

    REQUIRE(result.has_value());
    REQUIRE(result->out == "ping");
}

ERS_TEST_CASE("large", "[async]") {
    std::string input(3 * 1024 * 1024, 'x');

    auto result = ers::sync_wait(ers::run_command("cat",
        { .input = input, .max_output = 4 * 1024 * 1024 }));

    REQUIRE(result.has_value());
    REQUIRE(result->out.size() == input.size());
    REQUIRE(result->out == input);
}

ERS_TEST_CASE("early", "[async]") {
    std::string input(2 * 1024 * 1024, 'y');

    auto result = ers::sync_wait(ers::run_command("true",
        { .input = input }));

    REQUIRE(result.has_value());
    REQUIRE(result->exit_code == 0);
}

ERS_TEST_CASE("silent", "[async]") {
    auto result = ers::sync_wait(ers::run_command("cat"));

    REQUIRE(result.has_value());
    REQUIRE(result->out.empty());
}


// Killing ---------------------------------------------------------------------

ERS_TEST_CASE("timeout", "[async]") {
    auto begin = clock_type::now();
    auto result = ers::sync_wait(ers::run_command("sleep 30",
        { .timeout = 100ms, .grace = 100ms }));

    REQUIRE(mentions(result, "timed out"));
    REQUIRE(clock_type::now() - begin < 5s);
}

ERS_TEST_CASE("stubborn", "[async]") {
    test::TempTree tree;
    auto path = tree.path("pid");

    auto script = std::format("trap '' TERM; echo $$ > {}; i=0; while [ $i -lt 60 ]; do sleep 1; i=$((i+1)); done",
        path.string());

    auto result = ers::sync_wait(shell(script, { .timeout = 500ms, .grace = 100ms }));

    REQUIRE(mentions(result, "timed out"));
    REQUIRE(gone(std::stoi(tree.read("pid"))));
}

ERS_TEST_CASE("group", "[async]") {
    auto path = scratch("group");
    fs::remove(path);

    auto script = std::format("sleep 30 & echo $! > {}; wait",
        path.string());

    auto result = ers::sync_wait(shell(script, { .timeout = 500ms, .grace = 100ms }));

    REQUIRE(mentions(result, "timed out"));
    REQUIRE(gone(read_pid(path)));

    fs::remove(path);
}

ERS_TEST_CASE("orphan", "[async]") {
    auto path = scratch("orphan");
    fs::remove(path);

    auto script = std::format("sleep 30 & echo $! > {}",
        path.string());

    auto begin = clock_type::now();
    auto result = ers::sync_wait(shell(script));

    REQUIRE(result.has_value());
    REQUIRE(clock_type::now() - begin < 5s);
    REQUIRE(gone(read_pid(path)));

    fs::remove(path);
}


// Cancellation ----------------------------------------------------------------

ERS_TEST_CASE("cancel", "[async]") {
    auto path = scratch("cancel");
    fs::remove(path);

    ers::Scheduler scheduler({ .threads = 2 });

    auto script = std::format("echo $$ > {}; exec sleep 30",
        path.string());

    auto job = scheduler.spawn(shell(script, { .grace = 100ms }),
        { .interruptible = true });

    REQUIRE(wait_for_file(path));

    auto begin = clock_type::now();

    REQUIRE(job.cancel());

    auto result = job.get();

    REQUIRE(mentions(result, "cancelled"));
    REQUIRE(clock_type::now() - begin < 5s);
    REQUIRE(gone(read_pid(path)));

    fs::remove(path);
}

ERS_TEST_CASE("halt", "[async]") {
    auto path = scratch("halt");
    fs::remove(path);

    ers::Scheduler scheduler({ .threads = 2 });

    auto script = std::format("echo $$ > {}; exec sleep 30",
        path.string());

    auto job = scheduler.spawn(shell(script, { .grace = 100ms }),
        { .interruptible = true });

    REQUIRE(wait_for_file(path));

    scheduler.stop(5s);

    REQUIRE(job.ready());
    REQUIRE(mentions(job.get(), "cancelled"));
    REQUIRE(gone(read_pid(path)));

    fs::remove(path);
}
