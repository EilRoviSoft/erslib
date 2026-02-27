#pragma once

// std
#include <chrono>
#include <source_location>
#include <string>

// ers
#include <erslib/convert/string.hpp>
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
struct ers::to_sv<ers::Severity> {
    constexpr std::string_view operator()(const Severity& what) const noexcept {
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
};


// Error

namespace ers {
    class ERSLIB_EXPORT Error {
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
struct ers::to_str<ers::Error> {
    std::string operator()(const Error& what) const;
};
