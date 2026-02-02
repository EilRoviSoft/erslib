#pragma once

// std
#include <chrono>
#include <format>
#include <source_location>
#include <string>
#include <variant>

// ers
#include <erslib/type/time.hpp>
#include <erslib/convert/string.hpp>

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
constexpr std::string_view ers::to_sv<ers::Severity>(const Severity& what) {
    switch (what) {
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

// Error

namespace ers {
    class Error {
    public:
        Error(
            Severity severity,
            std::string code,
            std::string message,
            timestamp_t timestamp = std::chrono::system_clock::now(),
            const std::source_location& location = std::source_location::current()
        );
        virtual ~Error();

        Severity severity() const noexcept;
        std::string_view code() const noexcept;
        timestamp_t timestamp() const noexcept;
        std::string_view message() const noexcept;
        const std::source_location& location() const noexcept;

        // TODO: make fmt string static
        virtual std::string to_string() const;

    protected:
        Severity m_severity;
        std::string m_code;
        std::string m_message;
        timestamp_t m_timestamp;
        std::source_location m_location;
    };
}

template<>
std::string ers::to_string<ers::Error>(const Error& what);
