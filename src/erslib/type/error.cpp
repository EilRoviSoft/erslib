#include "erslib/type/error.hpp"


// ers
#include <erslib/type/exception.hpp>


ers::Error::Error(Severity severity, std::string_view code, std::string_view message, timestamp_t timestamp, const std::source_location& location) :
    m_severity(severity),
    m_code(code),
    m_message(message),
    m_timestamp(timestamp),
    m_location(location) {
}

ers::Error::Error(Error&& other) noexcept :
    m_severity(other.m_severity),
    m_code(std::move(other.m_code)),
    m_message(std::move(other.m_message)),
    m_timestamp(other.m_timestamp),
    m_location(other.m_location) {
}
ers::Error& ers::Error::operator=(Error&& other) noexcept {
    m_severity = other.m_severity;
    m_code = std::move(other.m_code);
    m_message = std::move(other.m_message);
    m_timestamp = other.m_timestamp;
    m_location = other.m_location;

    return *this;
}

ers::Error::~Error() = default;


ers::Error::operator Exception() const {
    return { *this };
}


ers::Severity ers::Error::severity() const noexcept { return m_severity; }
std::string_view ers::Error::code() const noexcept { return m_code; }
ers::timestamp_t ers::Error::timestamp() const noexcept { return m_timestamp; }
std::string_view ers::Error::message() const noexcept { return m_message; }
const std::source_location& ers::Error::location() const noexcept { return m_location; }


std::string ers::Error::to_string() const {
    return std::format(
        "[{:%Y-%m-%dT%H:%M:%S}] [{}] {}: {}\n\tat {}:{}",
        std::chrono::floor<std::chrono::seconds>(m_timestamp),
        convert::to_sv<Severity>(m_severity),
        m_code,
        m_message,
        m_location.file_name(),
        m_location.line()
    );
}
