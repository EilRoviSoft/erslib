#include "erslib/core/type/diagnostic.hpp"

// std
#include <ranges>


using std::string_view_literals::operator ""sv;


std::string ers::impl::Diagnostic::_build_message() const {
    return std::views::concat(m_context | std::views::reverse, std::views::single(m_message))
        | std::views::join_with(": "sv)
        | std::ranges::to<std::string>();
}

ers::impl::Diagnostic::Diagnostic(
    ESeverity severity,
    std::string message,
    timestamp_t timestamp
#ifdef _HAS_CPPTRACE
    , cpptrace::raw_trace trace
#endif
) :
    m_severity(severity),
    m_message(std::move(message)),
    m_timestamp(timestamp),
    m_trace(std::move(trace)) {
}


std::string ers::impl::Diagnostic::to_string(bool trim) const {
    return trim
        ? _build_message()
        : std::format(
            "[{:%Y-%m-%dT%H:%M:%S}] [{}] {}",
            std::chrono::floor<std::chrono::seconds>(m_timestamp),
            severity_name(m_severity),
#ifdef _HAS_CPPTRACE
            extend_with_trace(_build_message(), m_trace)
#else
            _build_message()
#endif
        );
}


ers::impl::Diagnostic&& ers::impl::Diagnostic::extend(std::string_view message) && {
    m_message += message;
    return std::move(*this);
}
