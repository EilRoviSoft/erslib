#pragma once

// ers
#include <erslib/flux/expr/ops.hpp>


// Implementation --------------------------------------------------------------

namespace flux::impl::clauses {
    inline constexpr fn_t<"coalesce"> coalesce {};
    inline constexpr fn_t<"nullif"> nullif {};
    inline constexpr fn_t<"greatest"> greatest {};
    inline constexpr fn_t<"least"> least {};
    inline constexpr fn_t<"now"> now {};

    inline constexpr binary_op_t<"||"> concat {};
}
