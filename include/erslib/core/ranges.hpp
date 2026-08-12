#pragma once


// Includes

#include <erslib/core/ranges/filter.hpp>
#include <erslib/core/ranges/util.hpp>


// Exports

namespace ers::views {
    using impl::views::filter;

    using impl::views::mem_filter;
    using impl::views::mem_transform;
}
