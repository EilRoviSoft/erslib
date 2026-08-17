#pragma once


// Includes

#include <erslib/core/meta/enum.hpp>
#include <erslib/core/meta/type_hash.hpp>
#include <erslib/core/meta/type_literal.hpp>
#include <erslib/core/meta/type_name.hpp>


// Exports

namespace ers::meta {
    using impl::meta::enumerator_of;

    using impl::meta::has_attribute;

    using impl::meta::has_template_attribute;
    using impl::meta::get_template_attribute_value;

    using impl::meta::has_type_attribute;
    using impl::meta::get_type_attribute_arg;

    using impl::meta::has_type_attribute2;
    using impl::meta::get_type_attribute_arg2;
}

namespace ers::meta {
    using impl::meta::type_hash;
    using impl::meta::type_hash_v;

    using impl::meta::type_literal;
    using impl::meta::type_literal_v;

    using impl::meta::type_name;
    using impl::meta::type_name_v;
}
