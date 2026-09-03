#pragma once

// std
#include <chrono>
#include <format>
#include <string>

// ers
#include <erslib/core/exception/impl.hpp>
#include <erslib/core/type/time.hpp>

// export
#include <erslib/export.hpp>


// ESeverity

namespace ers::impl {
    enum class ESeverity {
        Debug,
        Info,
        Warning,
        Error,
        Crit
    };

    constexpr std::string_view severity_name(ESeverity value) noexcept {
        switch (value) {
            case ESeverity::Debug:   return "DEBUG";
            case ESeverity::Info:    return "INFO";
            case ESeverity::Warning: return "WARNING";
            case ESeverity::Error:   return "ERROR";
            case ESeverity::Crit:    return "CRIT";
        }

        return "UNKNOWN";
    }
}

namespace ers {
    using impl::ESeverity;
    using impl::severity_name;
}


// Error

#ifdef _HAS_CPPTRACE

// cpptrace
#include <cpptrace/cpptrace.hpp>


namespace ers::impl {
    class ERSLIB_EXPORT Diagnostic {
    public:
        // Member functions

        Diagnostic(
            ESeverity severity,
            std::string message,
            timestamp_t timestamp = std::chrono::system_clock::now(),
            cpptrace::raw_trace trace = get_trace({ .skip = 1 })
        );


        // Observers

        ESeverity severity() const noexcept { return m_severity; }
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
        ESeverity m_severity;
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
            ESeverity severity,
            std::string message,
            timestamp_t timestamp = std::chrono::system_clock::now()
        );


        // Observers

        ESeverity severity() const noexcept { return m_severity; }
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
        ESeverity m_severity;
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
        return Diagnostic(ers::ESeverity::SEVERITY, std::format(fmt, std::forward<Args>(args)...)); \
    } \
    \
    inline Diagnostic make_##NAME(std::string message) { \
        return Diagnostic(ers::ESeverity::SEVERITY, std::move(message)); \
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
