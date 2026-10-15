#include "erslib/core/type/uuid.hpp"

// boost
#include <boost/uuid/string_generator.hpp>
#include <boost/uuid/uuid_io.hpp>


// Implementation

std::string ers::impl::uuid_to_string(const uuid& what) {
    return boost::uuids::to_string(what);
}

ers::Result<boost::uuids::uuid> ers::impl::uuid_from_string(std::string_view text) try {
    return boost::uuids::string_generator {}(text.begin(), text.end());
} catch (const std::exception& e) {
    return make_error("'{}' is not a UUID, got an error: {}.",
        text, e.what());
}


// Formatter

std::string std::formatter<boost::uuids::uuid>::format(const ers::uuid& v, std::format_context& ctx) const {
    return ers::uuid_to_string(v);
}
