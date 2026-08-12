#pragma once


// Includes

#include <erslib/core/adaptor/transparent_base.hpp>
#include <erslib/core/adaptor/transparent_string.hpp>
#include <erslib/core/adaptor/ranges.hpp>


// Exports

namespace ers::adaptor {
    using impl::hash;
    using impl::member_hash;

    using impl::equal;
    using impl::member_equal;

    using impl::unary_op;
    using impl::member_unary_op;

    using impl::binary_op;
    using impl::member_binary_op;
}

namespace ers::adaptor {
    using impl::string_hash;
    using impl::member_string_hash;
}

namespace ers::adaptor {
    using impl::member_range;
}
