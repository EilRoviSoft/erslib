#pragma once

// std
#include <chrono>
#include <concepts>
#include <format>
#include <string>

// cpptrace
#include <cpptrace/cpptrace.hpp>

// ers
#include <erslib/convert/impl/to_str.hpp>
#include <erslib/type/time.hpp>

// export
#include <erslib/export.hpp>

#include "erslib/exception/internal.hpp"


// Severity

namespace ers {
    enum class Severity {
        Debug,
        Info,
        Warning,
        Error,
        Crit
    };
}

template<>
struct ers::convert::to_string_backend<ers::Severity> {
    constexpr std::string_view constexpr_value(const Severity& value) const noexcept {
        switch (value) {
            case Severity::Debug:
                return "DEBUG";
            case Severity::Info:
                return "INFO";
            case Severity::Warning:
                return "WARNING";
            case Severity::Error:
                return "ERROR";
            case Severity::Crit:
                return "CRIT";
            default:
                return "UNKNOWN";
        }
    }
};


// Error

namespace ers {
    class ERSLIB_EXPORT Error {
    public:
        // Ctors

        Error(
            Severity severity,
            std::string_view message,
            timestamp_t timestamp = std::chrono::system_clock::now(),
            cpptrace::raw_trace trace = internal::get_trace(1)
        );


        // Copy ctors

        Error(const Error& other) = default;
        Error& operator=(const Error& other) = default;


        // Move ctors

        Error(Error&& other) noexcept;
        Error& operator=(Error&& other) noexcept;


        // Dtor

        virtual ~Error();


        // Observers

        Severity severity() const noexcept { return m_severity; }
        timestamp_t timestamp() const noexcept { return m_timestamp; }
        std::string_view message() const noexcept { return m_message; }
        const cpptrace::raw_trace& stacktrace() const noexcept { return m_trace; }


        virtual std::string to_string(bool trim_service_information = false) const;


    protected:
        Severity m_severity;
        std::string m_message;
        timestamp_t m_timestamp;
        cpptrace::raw_trace m_trace;
    };


    template<typename... Args>
        requires (sizeof...(Args) >= 1)
    Error make_error(Severity severity, std::format_string<Args...> fmt, Args&&... args) {
        return Error(severity, std::format(fmt, std::forward<Args>(args)...));
    }

    inline Error make_error(Severity severity, std::string_view message) {
        return Error(severity, message);
    }
}
