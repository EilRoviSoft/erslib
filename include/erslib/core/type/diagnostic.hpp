#pragma once

// std
#include <chrono>
#include <format>
#include <string>

// ers
#include <erslib/core/convert/impl/to_str.hpp>
#include <erslib/core/exception/internal.hpp>
#include <erslib/core/type/time.hpp>

// export
#include <erslib/export.hpp>


// Severity

namespace ers::impl {
    enum class Severity {
        Debug,
        Info,
        Warning,
        Error,
        Crit
    };
}

namespace ers {
    using impl::Severity;
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

#ifdef _HAS_CPPTRACE

// cpptrace
#include <cpptrace/cpptrace.hpp>


namespace ers::impl {
    class ERSLIB_EXPORT Diagnostic {
    public:
        // Member functions

        Diagnostic(
            Severity severity,
            std::string message,
            timestamp_t timestamp = std::chrono::system_clock::now(),
            cpptrace::raw_trace trace = get_trace({ .skip = 1 })
        );


        // Observers

        Severity severity() const noexcept { return m_severity; }
        timestamp_t timestamp() const noexcept { return m_timestamp; }
        std::string_view message() const noexcept { return m_message; }
        const cpptrace::raw_trace& stacktrace() const noexcept { return m_trace; }


        std::string to_string(bool trim = false) const;


        // Modifiers

        Diagnostic&& extend(std::string_view message) &&;

        template<typename... Args>
            requires (sizeof...(Args) >= 1)
        Diagnostic&& extend(std::format_string<Args...> fmt, Args&&... args) && {
            m_message += std::format(fmt, std::forward<Args>(args)...);
            return std::move(*this);
        }


    protected:
        Severity m_severity;
        std::string m_message;
        timestamp_t m_timestamp;
        cpptrace::raw_trace m_trace;
    };
}

#else

namespace ers::impl {
    class ERSLIB_EXPORT Diagnostic {
    public:
        // Member functions

        Diagnostic(
            Severity severity,
            std::string message,
            timestamp_t timestamp = std::chrono::system_clock::now()
        );


        // Observers

        Severity severity() const noexcept { return m_severity; }
        timestamp_t timestamp() const noexcept { return m_timestamp; }
        std::string_view message() const noexcept { return m_message; }


        std::string to_string(bool trim = false) const;


        // Modifiers

        Diagnostic&& extend(std::string_view message) &&;

        template<typename... Args>
            requires (sizeof...(Args) >= 1)
        Diagnostic&& extend(std::format_string<Args...> fmt, Args&&... args) && {
            m_message += std::format(fmt, std::forward<Args>(args)...);
            return std::move(*this);
        }


    protected:
        Severity m_severity;
        std::string m_message;
        timestamp_t m_timestamp;
    };
}

#endif

namespace ers {
    using impl::Diagnostic;
}


#define DEFINE_DIAGNOSTIC(NAME, SEVERITY) \
    template<typename... Args> \
        requires (sizeof...(Args) >= 1) \
    Diagnostic make_##NAME(std::format_string<Args...> fmt, Args&&... args) { \
        return Diagnostic(ers::Severity::SEVERITY, std::format(fmt, std::forward<Args>(args)...)); \
    } \
    \
    inline Diagnostic make_##NAME(std::string message) { \
        return Diagnostic(ers::Severity::SEVERITY, std::move(message)); \
    }

namespace ers::impl {
    DEFINE_DIAGNOSTIC(debug, Debug);
    DEFINE_DIAGNOSTIC(info, Info);
    DEFINE_DIAGNOSTIC(warning, Warning);
    DEFINE_DIAGNOSTIC(error, Error);
    DEFINE_DIAGNOSTIC(crit, Crit);
}

#undef DEFINE_DIAGNOSTIC


// Exports

namespace ers {
    using impl::make_debug;
    using impl::make_info;
    using impl::make_warning;
    using impl::make_error;
    using impl::make_crit;
}
