#pragma once

// std
#include <exception>
#include <format>
#include <string>

// export
#include <erslib/export.hpp>


namespace ers::internal {
    static constexpr size_t default_skip = 2;

    std::string ERSLIB_EXPORT extend_with_trace(std::string_view message, size_t skip);
}


#define ERS_MAKE_EXCEPTION_TYPE_WITH_BASE(NAME, BASE) \
    struct NAME : BASE { \
        template<typename... Args> \
        NAME(std::format_string<Args...> fmt, Args&&... args) : \
            BASE(ers::internal::extend_with_trace( \
                std::format(fmt, std::forward<Args>(args)...), \
                ers::internal::default_skip \
            )) { \
        } \
        explicit NAME(std::string_view what_arg) : \
            BASE(ers::internal::extend_with_trace(what_arg, ers::internal::default_skip)) { \
        } \
        \
        NAME(const NAME& other) = default; \
        NAME& operator=(const NAME& other) = default; \
    }

#define ERS_MAKE_EXCEPTION_TYPE_WITH_ERS_BASE(NAME, BASE) \
    struct NAME : BASE { \
        using BASE::BASE; \
        NAME(const NAME& other) = default; \
        NAME& operator=(const NAME& other) = default; \
    }
