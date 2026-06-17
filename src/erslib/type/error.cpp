#include "erslib/type/error.hpp"

// ers
#include <erslib/exception/internal.hpp>


ers::Error::Error(Severity severity, std::string message, timestamp_t timestamp, cpptrace::raw_trace trace) :
    _storage(std::make_unique<storage_type>(
        severity,
        std::move(message),
        timestamp,
        std::move(trace)
    )) {
}

ers::Error::Error(const Error& other) :
    _storage(std::make_unique<storage_type>(*other._storage)) {
}
ers::Error& ers::Error::operator=(const Error& other) {
    if (&other != this)
        _storage = std::make_unique<storage_type>(*other._storage);
    return *this;
}

ers::Error::Error(Error&& other) noexcept :
    _storage(std::move(other._storage)) {
}
ers::Error& ers::Error::operator=(Error&& other) noexcept {
    _storage = std::move(other._storage);
    return *this;
}


std::string ers::Error::to_string(bool trim) const {
    return trim
        ? internal::extend_with_trace(_storage->message, _storage->trace)
        : std::format(
            "[{:%Y-%m-%dT%H:%M:%S}] [{}] {}",
            std::chrono::floor<std::chrono::seconds>(_storage->timestamp),
            convert::to_sv<Severity>(_storage->severity),
            internal::extend_with_trace(_storage->message, _storage->trace)
        );
}


ers::Error&& ers::Error::expand_context(std::string_view message) && {
    _storage->message += '\n';
    _storage->message += message;
    return std::move(*this);
}
