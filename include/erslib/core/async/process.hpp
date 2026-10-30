#pragma once

#if defined(_HAS_BOOST_ASIO) && defined(__linux__)


// std
#include <chrono>
#include <cstddef>
#include <string>
#include <vector>

// ers
#include <erslib/core/async/task.hpp>

// export
#include <erslib/export.hpp>


// Details ---------------------------------------------------------------------

namespace ers::impl {
    struct process_options_t {
        std::string input;
        std::chrono::milliseconds timeout { 0 };
        std::chrono::milliseconds grace { 2000 };
        size_t max_output = 1024 * 1024;
    };


    struct process_output_t {
        int exit_code = 0;
        std::string out;
        std::string err;
    };
}


// Implementation --------------------------------------------------------------

namespace ers::impl {
    ERSLIB_EXPORT AsyncTask<process_output_t> run_process(std::vector<std::string> args, process_options_t options = {});
    ERSLIB_EXPORT AsyncTask<process_output_t> run_command(std::string line, process_options_t options = {});
}


// Exports ---------------------------------------------------------------------

namespace ers {
    using impl::process_options_t;
    using impl::process_output_t;

    using impl::run_process;
    using impl::run_command;
}


#endif
