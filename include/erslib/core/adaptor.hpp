#pragma once


// Includes

#include <erslib/core/adaptor/transparent_base.hpp>
#include <erslib/core/adaptor/transparent_string.hpp>
#include <erslib/core/adaptor/ranges.hpp>


// Exports

namespace ers::adaptor {
    using impl::adaptor::hash;
    using impl::adaptor::member_hash;

    using impl::adaptor::equal;
    using impl::adaptor::member_equal;

    using impl::adaptor::unary_op;
    using impl::adaptor::member_unary_op;

    using impl::adaptor::binary_op;
    using impl::adaptor::member_binary_op;
}

namespace ers::adaptor {
    using impl::adaptor::string_hash;
    using impl::adaptor::member_string_hash;
}

namespace ers::adaptor {
    using impl::adaptor::member_range;
}
