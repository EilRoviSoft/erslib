#pragma once

// std
#include <string>

// quill
#include <quill/Logger.h>
#include <quill/core/LogLevel.h>

// ers
#include <erslib/core/filesystem.hpp>
#include <erslib/core/type/general.hpp>

// export
#include <erslib/export.hpp>


namespace ers::logging::impl {
    struct logger_options_t {
        std::string name = "main";
        fs::path file = "logs/latest.log";
        std::string pattern = "%(time) [%(thread_id)] [%(log_level)] %(message)";
        std::string timestamp = "%H:%M:%S";
        size_t max_file_mib = 5;
        u32 max_files = 10;
        bool mirror_to_console = true;
        quill::LogLevel level = quill::LogLevel::TraceL1;
    };


    quill::Logger* ERSLIB_EXPORT make_logger(const logger_options_t& options = {});
}

namespace ers::logging {
    using impl::logger_options_t;
    using impl::make_logger;
}
