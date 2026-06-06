#pragma once

// std
#include <exception>
#include <format>
#include <string>


#define ERS_MAKE_EXCEPTION_FN(NAME, TYPE) \
    template<typename... Args> \
        requires (sizeof...(Args) >= 1) \
    TYPE NAME(std::format_string<Args...> fmt, Args&&... args) { \
        return TYPE(std::format(fmt, std::forward<Args>(args)...)); \
    } \
    inline TYPE NAME(std::string_view message) { \
        return TYPE(static_cast<std::string>(message)); \
    }

#define ERS_MAKE_EXCEPTION_TYPE(NAME) \
    struct NAME : std::exception { \
        template<typename... Args> \
        NAME(std::format_string<Args...> fmt, Args&&... args) : _what(std::format(fmt, std::forward<Args>(args)...)) {} \
        NAME(std::string_view what_arg) : _what(what_arg) {} \
        NAME(const NAME& other) = default; \
        NAME& operator=(const NAME& other) = default; \
        const char* what() const noexcept { return _what.c_str(); } \
    private: \
        std::string _what; \
    }

#define ERS_MAKE_EXCEPTION_TYPE_WITH_BASE(NAME, BASE) \
    struct NAME : BASE { \
        template<typename... Args> \
        NAME(std::format_string<Args...> fmt, Args&&... args) : BASE(std::format(fmt, std::forward<Args>(args)...)) {} \
        NAME(std::string_view what_arg) : BASE(what_arg.data()) {} \
        NAME(const NAME& other) = default; \
        NAME& operator=(const NAME& other) = default; \
        const char* what() const noexcept { return BASE::what(); } \
    }
