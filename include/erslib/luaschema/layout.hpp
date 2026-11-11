#pragma once


// Goal is to have something like this:
// { "energy-source", required() | expected_types<ElectricEnergySource, VoidEnergySource> },
// { "active-energy-usage", required() | expected_type<Energy> }
// { "icons", expected_type<vector<IconData>> | exclusive_with({ "icon", "icon-size" }) }
// { "icon", expected_type<FileName>() | exclusive_with("icons") }
// { "icon-size , expected_type<SpriteSizeType> | exclusive_with("icons") }


// In future this can be done with reflection:
// [[=reflect::required]]
// std::string type;


// Includes --------------------------------------------------------------------

#include <erslib/luaschema/impl/field.hpp>
#include <erslib/luaschema/impl/layout.hpp>
#include <erslib/luaschema/impl/parser.hpp>
#include <erslib/luaschema/impl/verifier.hpp>

#include <erslib/luaschema/impl/util.hpp>


// Utility

#include <erslib/luaschema/verifiers/innate.hpp>
#include <erslib/luaschema/verifiers/user_defined.hpp>
#include <erslib/luaschema/verifiers/vector.hpp>

#include <erslib/luaschema/parsers/innate.hpp>
#include <erslib/luaschema/parsers/user_defined.hpp>
#include <erslib/luaschema/parsers/vector.hpp>


// Properties

#include <erslib/luaschema/properties/exclusive_with.hpp>
#include <erslib/luaschema/properties/inclusive_with.hpp>
#include <erslib/luaschema/properties/one_of.hpp>
#include <erslib/luaschema/properties/parse_into.hpp>
#include <erslib/luaschema/properties/parse_into_with_index.hpp>
#include <erslib/luaschema/properties/presence.hpp>
#include <erslib/luaschema/properties/with_types.hpp>


// Exports ---------------------------------------------------------------------

namespace ers::luaschema {
    namespace properties = impl::luaschema::properties;


    using impl::luaschema::verify_context;
    using impl::luaschema::parser_context;

    using impl::luaschema::IVerifier;
    using impl::luaschema::Verifier;

    using impl::luaschema::IParser;
    using impl::luaschema::Parser;

    using impl::luaschema::IDescriptor;
    using impl::luaschema::Descriptor;

    using impl::luaschema::Field;

    using impl::luaschema::Layout;
    using impl::luaschema::LayoutContainer;


    using impl::luaschema::required_field;
    using impl::luaschema::optional_field;
}
