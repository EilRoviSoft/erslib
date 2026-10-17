#pragma once

// ers
#include <erslib/flux/reflect/types.hpp>


namespace flux::impl::reflect {
    inline constexpr auto now = raw_sql<"now()">;
    inline constexpr auto current_date = raw_sql<"CURRENT DATE">;
}
