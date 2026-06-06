#include "erslib/type/error.hpp"


ers::Error::Error(Severity severity, std::string_view message, timestamp_t timestamp, cpptrace::stacktrace stacktrace) :
    m_severity(severity),
    m_message(message),
    m_timestamp(timestamp),
    m_stacktrace(std::move(stacktrace)) {
}

ers::Error::Error(Error&& other) noexcept :
    m_severity(other.m_severity),
    m_message(std::move(other.m_message)),
    m_timestamp(other.m_timestamp),
    m_stacktrace(std::move(other.m_stacktrace)) {
}
ers::Error& ers::Error::operator=(Error&& other) noexcept {
    m_severity = other.m_severity;
    m_message = std::move(other.m_message);
    m_timestamp = other.m_timestamp;
    m_stacktrace = other.m_stacktrace;

    return *this;
}

ers::Error::~Error() = default;

std::string ers::Error::to_string(bool trim_service_information) const {
    return trim_service_information
    ? std::format("{}\n{}", m_message, m_stacktrace.frames[1].to_string())
    : std::format(
        "[{:%Y-%m-%dT%H:%M:%S}] [{}] {}\n{}",
        std::chrono::floor<std::chrono::seconds>(m_timestamp),
        convert::to_sv<Severity>(m_severity),
        m_message,
        m_stacktrace.frames[1].to_string()
    );
}
