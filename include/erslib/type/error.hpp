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
            cpptrace::stacktrace stacktrace = cpptrace::generate_trace()
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
        const cpptrace::stacktrace& stacktrace() const noexcept { return m_stacktrace; }


        // TODO: make fmt string static
        virtual std::string to_string() const;


    protected:
        Severity m_severity;
        std::string m_message;
        timestamp_t m_timestamp;
        cpptrace::stacktrace m_stacktrace;
    };


    template<typename... Args>
    Error make_error(Severity severity, std::string_view fmt, Args&&... args) {
        return Error(severity, std::vformat(fmt, std::make_format_args(args...)));
    }
}
