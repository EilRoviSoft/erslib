#include "erslib/exception/internal.hpp"

// cpptrace
#include <cpptrace/formatting.hpp>


namespace {
    auto make_formatter() {
        using namespace cpptrace;

        return formatter()
            .colors(formatter::color_mode::automatic)
            .symbols(formatter::symbol_mode::pruned);
    }

    auto get_formatter() {
        static cpptrace::formatter formatter = make_formatter();
        return formatter;
    }
}


cpptrace::raw_trace ers::internal::get_trace(size_t skip) {
#if !defined(ERS_TRACE_VERBOSITY) || ERS_TRACE_VERBOSITY == 0

    return cpptrace::raw_trace();

#elif ERS_TRACE_VERBOSITY == 1

    return cpptrace::generate_raw_trace(skip + 1, 1);

#elif ERS_TRACE_VERBOSITY == 2

#if ERS_TRACE_MAX_DEPTH != 0

    return cpptrace::generate_raw_trace(skip + 1, ERS_TRACE_MAX_DEPTH);

#else

    return cpptrace::generate_raw_trace(skip + 1);

#endif

#endif
}


std::string ers::internal::extend_with_trace(std::string_view message, const cpptrace::raw_trace& trace) {
#if !defined(ERS_TRACE_VERBOSITY) || ERS_TRACE_VERBOSITY == 0

    return static_cast<std::string>(message);

#else

    return std::format("{}\n{}", message, get_formatter().format(trace.resolve()));

#endif
}

std::string ers::internal::extend_with_trace(std::string_view message, size_t skip) {
#if !defined(ERS_TRACE_VERBOSITY) || ERS_TRACE_VERBOSITY == 0

    return static_cast<std::string>(message);

#else

    return std::format("{}\n{}", message, get_formatter().format(get_trace(skip).resolve()));

#endif
}
