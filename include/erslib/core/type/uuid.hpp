#pragma once

#ifdef _HAS_BOOST_UUID

// std
#include <cstddef>
#include <format>
#include <string>
#include <string_view>

// boost
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_hash.hpp>

// ers
#include <erslib/core/hashing/base.hpp>
#include <erslib/core/type/maybe.hpp>
#include <erslib/core/type/result.hpp>

// export
#include <erslib/export.hpp>


namespace ers::impl {
    using uuid = boost::uuids::uuid;


    ERSLIB_EXPORT std::string uuid_to_string(const uuid& what);

    ERSLIB_EXPORT uuid uuid_from_string(std::string_view text);
    ERSLIB_EXPORT Result<uuid> try_uuid_from_string(std::string_view text);
    ERSLIB_EXPORT maybe<uuid> maybe_uuid_from_string(std::string_view text);
}


// Exports ---------------------------------------------------------------------

namespace ers {
    using impl::uuid;


    using impl::uuid_to_string;

    using impl::uuid_from_string;
    using impl::try_uuid_from_string;
    using impl::maybe_uuid_from_string;
}


// Specializations -------------------------------------------------------------

// Hashing

template<typename Policy>
struct ers::THashBase<ers::uuid, Policy> {
    using type = uuid;

    size_t operator()(
        const type& what, size_t seed = 0
    ) const noexcept requires (hashing::RawBytesBackend<Policy>) {
        const auto bytes = reinterpret_cast<const std::byte*>(what.begin());
        return hashing::backend<Policy>::process_raw_bytes({ bytes, what.size() }, seed);
    }

    size_t operator()(
        const type& what, size_t seed = 0
    ) const noexcept requires (!hashing::RawBytesBackend<Policy>) {
        return hashing::backend<Policy>::process_value(what, seed);
    }
};


// Formatter

template<>
struct std::formatter<ers::uuid> {
    // {:[open][close]}
    constexpr auto parse(std::format_parse_context& ctx) {
        return ctx.begin();
    }

    ERSLIB_EXPORT std::string format(const ers::uuid& v, std::format_context& ctx) const;
};


#endif
