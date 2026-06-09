#pragma once

// std
#include <limits>
#include <exception>
#include <format>
#include <string>

// cpptrace
#include <cpptrace/cpptrace.hpp>

// export
#include <erslib/export.hpp>


namespace ers::internal {
#if !defined(ERS_TRACE_VERBOSITY) || ERS_TRACE_VERBOSITY == 0

#elif ERS_TRACE_VERBOSITY == 1

    static constexpr size_t max_depth_default = 1;

#elif ERS_TRACE_VERBOSITY == 2

#if ERS_TRACE_MAX_DEPTH != 0

    static constexpr size_t max_depth_default = ERS_TRACE_MAX_DEPTH;

#else

    static constexpr size_t max_depth_default = std::numeric_limits<size_t>::max();

#endif

#endif
}


namespace ers {
    struct trace_config_t {
        size_t skip = 2;
        size_t max_depth = internal::max_depth_default;
    };
}


namespace ers::internal {
    cpptrace::raw_trace ERSLIB_EXPORT get_trace(trace_config_t config = {});

    std::string ERSLIB_EXPORT extend_with_trace(std::string_view message, const cpptrace::raw_trace& trace);
    std::string ERSLIB_EXPORT extend_with_trace(std::string_view message, trace_config_t config);
}


#define ERS_MAKE_EXCEPTION_FUNCTOR(NAME, TYPE) \
    struct NAME##_fn { \
        template<typename... Args> \
        TYPE operator()(std::format_string<Args...> fmt, Args&&... args) const { \
            return TYPE(std::format(fmt, std::forward<Args>(args)...)); \
        } \
        template<typename... Args> \
        TYPE operator()(ers::trace_config_t config, std::format_string<Args...> fmt, Args&&... args) const { \
            return TYPE(ers::internal::extend_with_trace(std::format(fmt, std::forward<Args>(args)...), config)); \
        } \
        \
        TYPE operator()(std::string_view what_arg) const { \
            return TYPE(what_arg.data()); \
        } \
        TYPE operator()(ers::trace_config_t config, std::string_view what_arg) const { \
            return TYPE(ers::internal::extend_with_trace(what_arg, config)); \
        } \
    }; \
    static constexpr NAME##_fn make_##NAME


#define ERS_MAKE_EXCEPTION_TYPE(NAME, BASE) \
    struct NAME : BASE { \
        explicit NAME(const std::string& what_arg) : \
            BASE(what_arg) { \
        } \
        \
        explicit NAME(const char* what_arg) : \
            BASE(what_arg) { \
        } \
        \
        NAME(const NAME&) noexcept = default; \
        NAME& operator=(const NAME& other) noexcept = default; \
    }; \
    \
    ERS_MAKE_EXCEPTION_FUNCTOR(NAME, NAME)
