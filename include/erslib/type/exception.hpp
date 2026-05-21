#pragma once

// std
#include <exception>
#include <format>

// ers
#include <erslib/meta/type_name.hpp>
#include <erslib/type/error.hpp>

// export
#include <erslib/export.hpp>


namespace ers {
    class ERSLIB_EXPORT Exception : public std::exception, Error {
    public:
        Exception(
            Severity severity,
            std::string_view code,
            std::string_view message,
            timestamp_t timestamp = std::chrono::system_clock::now(),
            cpptrace::stacktrace stacktrace = cpptrace::generate_trace()
        );

        Exception(Error error);


        const char* what() const override;


        const Error& to_error() const;


    private:
        mutable bool _needs_initialization = true;
        mutable std::string _what;


        void _init() const;
    };


    template<typename T, typename... Args>
        requires std::derived_from<T, std::exception>
    Exception make_exception(Severity severity, const std::format_string<Args...>& fmt, Args&&... args) {
        return Exception(severity, meta::type_name_v<T>, std::format(fmt, std::forward<Args>(args)...));
    }

    template<typename T, typename... Args>
        requires std::derived_from<T, std::exception>
    Exception make_exception(const std::format_string<Args...>& fmt, Args&&... args) {
        return make_exception<T>(Severity::Error, fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    Exception make_exception(Severity severity, const std::format_string<Args...>& fmt, Args&&... args) {
        return make_exception<std::runtime_error>(severity, fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    Exception make_exception(const std::format_string<Args...>& fmt, Args&&... args) {
        return make_exception(Severity::Error, fmt, std::forward<Args>(args)...);
    }
}
