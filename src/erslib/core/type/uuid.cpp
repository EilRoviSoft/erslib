#include "erslib/core/type/uuid.hpp"

// std
#include <concepts>
#include <exception>

// boost
#include <boost/uuid/string_generator.hpp>
#include <boost/uuid/uuid_io.hpp>


// Implementation --------------------------------------------------------------

std::string ers::impl::uuid_to_string(const uuid& what) {
    return boost::uuids::to_string(what);
}


boost::uuids::uuid ers::impl::uuid_from_string(std::string_view text) {
    return boost::uuids::string_generator {}(text.begin(), text.end());
}

ers::Result<boost::uuids::uuid> ers::impl::try_uuid_from_string(std::string_view text) try {
    return uuid_from_string(text);
} catch (const std::exception& e) {
    return make_error("{}", e.what());
}

ers::maybe<boost::uuids::uuid> ers::impl::maybe_uuid_from_string(std::string_view text) try {
    return uuid_from_string(text);
} catch (...) {
    return none;
}


// Specializations -------------------------------------------------------------

std::string std::formatter<boost::uuids::uuid>::format(const ers::uuid& v, std::format_context& /*ctx*/) const {
    return ers::uuid_to_string(v);
}
