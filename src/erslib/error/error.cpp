#include <erslib/error/error.hpp>

ers::Error::Error(Severity severity, std::string code, std::string message, timestamp_t timestamp, const std::source_location& location) :
	m_severity(severity),
	m_code(std::move(code)),
	m_message(std::move(message)),
	m_timestamp(timestamp),
	m_location(location) {
}
ers::Error::~Error() = default;

ers::Severity ers::Error::severity() const noexcept { return this->m_severity; }
std::string_view ers::Error::code() const noexcept { return this->m_code; }
ers::timestamp_t ers::Error::timestamp() const noexcept { return this->m_timestamp; }
std::string_view ers::Error::message() const noexcept { return this->m_message; }
const std::source_location& ers::Error::location() const noexcept { return this->m_location; }

std::string ers::Error::to_string() const {
	return std::format("[{:%Y-%m-%dT%H:%M:%S}] [{}] {}: {}\n\tat {}:{}",
		std::chrono::floor<std::chrono::seconds>(this->m_timestamp),
		util::to_sv(this->m_severity),
		this->m_code,
		this->m_message,
		this->m_location.file_name(),
		this->m_location.line()
	);
}
