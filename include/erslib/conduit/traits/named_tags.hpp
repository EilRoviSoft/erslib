#pragma once

// ers
#include <erslib/conduit/reflect/types.hpp>


namespace conduit::impl::reflect {
    inline constexpr auto now = raw_sql<"now()">;
    inline constexpr auto current_date = raw_sql<"CURRENT DATE">;
}
