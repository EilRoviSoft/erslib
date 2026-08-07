#pragma once

// std
#include <limits>
#include <format>
#include <string>

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


#ifdef _HAS_CPPTRACE

// cpptrace
#include <cpptrace/cpptrace.hpp>


namespace ers::internal {
    cpptrace::raw_trace ERSLIB_EXPORT get_trace(trace_config_t config = {});

    std::string ERSLIB_EXPORT extend_with_trace(std::string_view message, const cpptrace::raw_trace& raw_trace);
    std::string ERSLIB_EXPORT extend_with_trace(std::string_view message, trace_config_t config);
}


namespace ers::internal {
    template<bool IncludeTrace, typename Type>
    struct exception_fn;

    template<typename Type>
    struct exception_fn<true, Type> {
        template<typename... Args>
        Type operator()(trace_config_t config, std::format_string<Args...> fmt, Args&&... args) const {
            return Type(internal::extend_with_trace(std::format(fmt, std::forward<Args>(args)...), config));
        }
        template<typename... Args>
        Type operator()(std::format_string<Args...> fmt, Args&&... args) const {
            return Type(internal::extend_with_trace(std::format(fmt, std::forward<Args>(args)...), trace_config_t {}));
        }

        Type operator()(trace_config_t config, std::string_view what_arg) const {
            return Type(extend_with_trace(what_arg, config));
        }
        Type operator()(std::string_view what_arg) const {
            return Type(extend_with_trace(what_arg, trace_config_t {}));
        }
    };

    template<typename Type>
    struct exception_fn<false, Type> {
        template<typename... Args>
        Type operator()(std::format_string<Args...> fmt, Args&&... args) const {
            return Type(std::format(fmt, std::forward<Args>(args)...));
        }
        Type operator()(std::string_view what_arg) const {
            return Type(what_arg.data());
        }
    };
}

#else

namespace ers::internal {
    template<bool IncludeTrace, typename Type>
    struct exception_fn;

    template<typename Type>
    struct exception_fn<true, Type> {
        template<typename... Args>
        Type operator()(trace_config_t, std::format_string<Args...> fmt, Args&&... args) const {
            return Type(std::format(fmt, std::forward<Args>(args)...));
        }
        template<typename... Args>
        Type operator()(std::format_string<Args...> fmt, Args&&... args) const {
            return Type(std::format(fmt, std::forward<Args>(args)...));
        }

        Type operator()(trace_config_t, std::string_view what_arg) const {
            return Type(what_arg.data());
        }
        Type operator()(std::string_view what_arg) const {
            return Type(what_arg.data());
        }
    };

    template<typename Type>
    struct exception_fn<false, Type> {
        template<typename... Args>
        Type operator()(std::format_string<Args...> fmt, Args&&... args) const {
            return Type(std::format(fmt, std::forward<Args>(args)...));
        }
        Type operator()(std::string_view what_arg) const {
            return Type(what_arg.data());
        }
    };
}

#endif


#define ERS_MAKE_EXCEPTION_FUNCTOR(NAME, TYPE) \
    template<bool IncludeTrace> \
    struct NAME##_fn : ers::internal::exception_fn<IncludeTrace, TYPE> { \
        using base_type = ers::internal::exception_fn<IncludeTrace, TYPE>; \
        using base_type::operator(); \
    }; \
    static constexpr NAME##_fn<false> make_##NAME; \
    static constexpr NAME##_fn<true> make_##NAME##_with_trace

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
