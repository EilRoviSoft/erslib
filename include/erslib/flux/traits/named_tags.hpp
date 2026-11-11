#pragma once

// ers
#include <erslib/flux/reflect/types.hpp>


namespace flux::impl::reflect::named_tags {
    inline constexpr auto now = raw_sql<"now()">;
    inline constexpr auto current_date = raw_sql<"CURRENT_DATE">;
    inline constexpr auto empty_tsvector = raw_sql<"''::tsvector">;
}
