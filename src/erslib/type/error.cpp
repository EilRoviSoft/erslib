#include "erslib/type/error.hpp"

// ers
#include <erslib/exception/internal.hpp>


ers::Error::Error(Severity severity, std::string_view message, timestamp_t timestamp, cpptrace::raw_trace trace) :
    m_severity(severity),
    m_message(message),
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

ers::Error::~Error() = default;

std::string ers::Error::to_string(bool trim_service_information) const {
    return trim_service_information
        ? internal::extend_with_trace(m_message, m_trace)
        : std::format(
            "[{:%Y-%m-%dT%H:%M:%S}] [{}] {}",
            std::chrono::floor<std::chrono::seconds>(m_timestamp),
            convert::to_sv<Severity>(m_severity),
            internal::extend_with_trace(m_message, m_trace)
        );
}
