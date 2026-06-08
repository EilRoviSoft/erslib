#include "erslib/exception/internal.hpp"

// cpptrace
#include <cpptrace/cpptrace.hpp>


std::string ers::internal::extend_with_trace(std::string_view message, size_t skip) {
#if !defined(ERS_TRACE_VERBOSITY) || ERS_TRACE_VERBOSITY == 0

    return static_cast<std::string>(message);

#elif ERS_TRACE_VERBOSITY == 1

    auto trace = cpptrace::generate_trace(skip);
    return std::format("{}\n{}", message, trace.frames.front().to_string());

#elif ERS_TRACE_VERBOSITY == 2

    auto trace = cpptrace::generate_trace(skip);

#if ERS_TRACE_MAX_DEPTH != 0

    if (trace.frames.size() > ERS_TRACE_MAX_DEPTH)
        trace.frames.erase(trace.frames.begin() + ERS_TRACE_MAX_DEPTH, trace.frames.end());

#endif

    return std::format("{}\n{}", message, trace.to_string());

#endif
}
