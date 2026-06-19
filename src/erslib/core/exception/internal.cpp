#include "erslib/core/exception/internal.hpp"

// std
#include <ranges>

// cpptrace
#include <cpptrace/formatting.hpp>


namespace {
    auto& get_formatter() {
        using namespace cpptrace;

        static auto value = formatter()
            .colors(formatter::color_mode::automatic)
            .paths(formatter::path_mode::basename)
            .symbols(formatter::symbol_mode::pruned);

        return value;
    }
}


namespace {
    auto resolve_and_format(const cpptrace::raw_trace& raw_trace) {
        std::string result;

        if (raw_trace.frames.empty())
            return result;

        auto trace = raw_trace.resolve();

        {
            auto& e = trace.frames.front();
            result = cpptrace::get_snippet(e.filename, e.line.value(), e.column, 2);
        }

        result += "\n" + get_formatter().format(trace);

        return result;
    }
}


cpptrace::raw_trace ers::internal::get_trace(trace_config_t config) {
#if !defined(ERS_TRACE_VERBOSITY) || ERS_TRACE_VERBOSITY == 0

    return cpptrace::raw_trace();

#else

    return cpptrace::generate_raw_trace(config.skip + 1, config.max_depth);

#endif
}


std::string ers::internal::extend_with_trace(std::string_view message, const cpptrace::raw_trace& raw_trace) {
#if !defined(ERS_TRACE_VERBOSITY) || ERS_TRACE_VERBOSITY == 0

    return static_cast<std::string>(message);

#else

    return std::format("{}\n{}", message, resolve_and_format(raw_trace));

#endif
}

std::string ers::internal::extend_with_trace(std::string_view message, trace_config_t config) {
#if !defined(ERS_TRACE_VERBOSITY) || ERS_TRACE_VERBOSITY == 0

    return static_cast<std::string>(message);

#else

    return std::format("{}\n{}", message, resolve_and_format(get_trace(config)));

#endif
}
