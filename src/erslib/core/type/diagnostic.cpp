#include "erslib/core/type/diagnostic.hpp"

// ers
#include <erslib/core/exception/internal.hpp>


#ifdef _HAS_CPPTRACE

ers::impl::Diagnostic::Diagnostic(Severity severity, std::string message, timestamp_t timestamp, cpptrace::raw_trace trace) :
    m_severity(severity),
    m_message(std::move(message)),
    m_timestamp(timestamp),
    m_trace(std::move(trace)) {
}


std::string ers::impl::Diagnostic::to_string(bool trim) const {
    return trim
        ? extend_with_trace(m_message, m_trace)
        : std::format(
            "[{:%Y-%m-%dT%H:%M:%S}] [{}] {}",
            std::chrono::floor<std::chrono::seconds>(m_timestamp),
            convert::to_sv<Severity>(m_severity),
            extend_with_trace(m_message, m_trace)
        );
}


ers::impl::Diagnostic&& ers::impl::Diagnostic::extend(std::string_view message) && {
    m_message += message;
    return std::move(*this);
}

#else

ers::impl::Diagnostic::Diagnostic(Severity severity, std::string message, timestamp_t timestamp) :
    m_severity(severity),
    m_message(std::move(message)),
    m_timestamp(timestamp) {
}


std::string ers::impl::Diagnostic::to_string(bool trim) const {
    return trim
        ? m_message
        : std::format(
            "[{:%Y-%m-%dT%H:%M:%S}] [{}] {}",
            std::chrono::floor<std::chrono::seconds>(m_timestamp),
            convert::to_sv<Severity>(m_severity),
            m_message
        );
}


ers::impl::Diagnostic&& ers::impl::Diagnostic::extend(std::string_view message) && {
    m_message += message;
    return std::move(*this);
}

#endif
