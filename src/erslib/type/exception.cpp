#include "erslib/type/exception.hpp"


ers::Exception::Exception(
    Severity severity,
    std::string_view code,
    std::string_view message,
    timestamp_t timestamp,
    const std::source_location& location
) : Error(severity, code, message, timestamp, location) {
}

ers::Exception::Exception(Error error) :
    Error(std::move(error)) {
}


const char* ers::Exception::what() const {
    _init();
    return _what.c_str();
}

const ers::Error& ers::Exception::to_error() const {
    return static_cast<const Error&>(*this);
}


void ers::Exception::_init() const {
    if (_needs_initialization) {
        _what = Error::to_string();
        _needs_initialization = false;
    }
}
