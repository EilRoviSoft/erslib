#pragma once


// Includes --------------------------------------------------------------------

#include <erslib/contrib/json/impl.hpp>

#include <erslib/contrib/json/concept.hpp>
#include <erslib/contrib/json/convert.hpp>
#include <erslib/contrib/json/read.hpp>
#include <erslib/contrib/json/schema.hpp>
#include <erslib/contrib/json/traits.hpp>


// Exports ---------------------------------------------------------------------

namespace utl {
    using impl::EFormat;
    using impl::ENodeType;
    using impl::type_label;
    using impl::JsonTraits;


    using impl::TNode;

    using impl::default_traits;
    using impl::Json;

    using impl::ordered_traits;
    using impl::OrderedJson;

    using impl::JsonPointer;


    using impl::Object;
    using impl::Array;
    using impl::String;
    using impl::Integral;
    using impl::Floating;
    using impl::Bool;
    using impl::Null;


    using impl::from_string;
    using impl::from_file;
    using impl::try_from_string;
    using impl::try_from_file;

    using impl::read_as;

    using impl::literals::operator""_json;
}


namespace utl {
    using impl::IJsonConvertible;
    using impl::load_from_json;
    using impl::save_into_json;

    using impl::JsonCompatible;
    using impl::TSchema;
    using impl::JsonSchema;
    using impl::json_storage_t;
}


namespace utl {
    using impl::json_value;
    using impl::JsonNode;
    using impl::JsonConvertible;

    using impl::read;
    using impl::write;

    using impl::json_object;
    using json_key = impl::json_key_t;
}
