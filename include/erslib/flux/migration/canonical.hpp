#pragma once

// std
#include <string>
#include <string_view>

// ers
#include <erslib/core/type/general.hpp>


namespace flux::impl {
    std::string canonical_sql(std::string_view sql);
}
