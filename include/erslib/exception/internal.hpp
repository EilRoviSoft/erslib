#pragma once

// std
#include <exception>
#include <format>
#include <string>

// cpptrace
#include <cpptrace/cpptrace.hpp>

// export
#include <erslib/export.hpp>


namespace ers::internal {
    cpptrace::raw_trace ERSLIB_EXPORT get_trace(size_t skip = 0);

    std::string ERSLIB_EXPORT extend_with_trace(std::string_view message, const cpptrace::raw_trace& trace);
    std::string ERSLIB_EXPORT extend_with_trace(std::string_view message, size_t skip = 0);
}


#define ERS_MAKE_EXCEPTION_FUNCTOR(NAME, BASE) \
    struct NAME##_fn { \
        template<typename... Args> \
        BASE operator()(std::format_string<Args...> fmt, Args&&... args) const { \
            return BASE(ers::internal::extend_with_trace(std::format(fmt, std::forward<Args>(args)...), 2)); \
        } \
        template<typename... Args> \
        BASE operator()(size_t skip, std::format_string<Args...> fmt, Args&&... args) const { \
            return BASE(ers::internal::extend_with_trace(std::format(fmt, std::forward<Args>(args)...), 2 + skip)); \
        } \
        \
        BASE operator()(std::string_view what_arg) const { \
            return BASE(ers::internal::extend_with_trace(what_arg, 2)); \
        } \
        BASE operator()(size_t skip, std::string_view what_arg) const { \
            return BASE(ers::internal::extend_with_trace(what_arg, 2 + skip)); \
        } \
    }; \
    static constexpr NAME##_fn NAME

#define ERS_MAKE_EXCEPTION_TYPE_WITH_BASE(NAME, BASE) \
    struct NAME##_impl : BASE { \
        explicit NAME##_impl(std::string what_arg) : \
            BASE(std::move(what_arg)) { \
        } \
        \
    }; \
    ERS_MAKE_EXCEPTION_FUNCTOR(NAME, NAME##_impl)

#define ERS_MAKE_EXCEPTION_TYPE_WITH_ERS_BASE(NAME, BASE) \
    struct NAME##_impl : BASE##_impl { \
        explicit NAME##_impl(std::string what_arg) : \
            BASE##_impl(std::move(what_arg)) { \
        } \
        \
    }; \
    ERS_MAKE_EXCEPTION_FUNCTOR(NAME, NAME##_impl)
