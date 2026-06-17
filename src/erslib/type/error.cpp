#include "erslib/type/error.hpp"

// ers
#include <erslib/exception/internal.hpp>


ers::Error::Error(Severity severity, std::string message, timestamp_t timestamp, cpptrace::raw_trace trace) :
    m_severity(severity),
    m_message(std::move(message)),
    m_timestamp(timestamp),
    m_trace(std::move(trace)) {
}

ers::Error::Error(Error&& other) noexcept :
    m_severity(other.m_severity),
    m_message(std::move(other.m_message)),
    m_timestamp(other.m_timestamp),
    m_trace(std::move(other.m_trace)) {
}
ers::Error& ers::Error::operator=(Error&& other) noexcept {
    m_severity = other.m_severity;
    m_message = std::move(other.m_message);
    m_timestamp = other.m_timestamp;
    m_trace = other.m_trace;

    return *this;
}


std::string ers::Error::to_string(bool trim) const {
    return trim
        ? internal::extend_with_trace(m_message, m_trace)
        : std::format(
            "[{:%Y-%m-%dT%H:%M:%S}] [{}] {}",
            std::chrono::floor<std::chrono::seconds>(m_timestamp),
            convert::to_sv<Severity>(m_severity),
            internal::extend_with_trace(m_message, m_trace)
        );
}


ers::Error&& ers::Error::expand_context(std::string_view message) && {
    m_message += '\n';
    m_message += message;
    return std::move(*this);
}
