#pragma once


// Includes

#include <erslib/contrib/json/impl.hpp>

#include <erslib/contrib/json/concept.hpp>
#include <erslib/contrib/json/convert.hpp>
#include <erslib/contrib/json/schema.hpp>


// Exports

namespace utl {
    using impl::EFormat;

    using impl::Node;
    using Json = Node;

    using impl::Object;
    using impl::Array;
    using impl::String;
    using impl::Integral;
    using impl::Floating;
    using impl::Bool;
    using impl::Null;

    using impl::from_string;
    using impl::from_file;

    using impl::literals::operator ""_json;
}

namespace utl {
    using impl::IJsonConvertible;
    using impl::load_from_json;
    using impl::save_into_json;


    using impl::JsonCompatible;
    using impl::JsonSchema;
}
