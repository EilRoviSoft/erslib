#pragma once

#ifdef _HAS_BOOST_UUID

// ers
#include <erslib/contrib/json/traits/text.hpp>
#include <erslib/core/convert/triple.hpp>
#include <erslib/core/type/uuid.hpp>


template<>
struct utl::impl::json_text<ers::uuid> :
    ers::parse_triple<ers::uuid, &ers::try_uuid_from_string, &ers::maybe_uuid_from_string> {
    static std::string to_text(const ers::uuid& what) {
        return ers::uuid_to_string(what);
    }
};

#endif
