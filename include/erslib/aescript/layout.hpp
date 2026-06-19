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


// Main implementation

#include <erslib/aescript/impl/field.hpp>
#include <erslib/aescript/impl/layout.hpp>
#include <erslib/aescript/impl/parser.hpp>
#include <erslib/aescript/impl/verifier.hpp>

#include <erslib/aescript/impl/util.hpp>


// Utility

#include <erslib/aescript/verifiers/innate.hpp>
#include <erslib/aescript/verifiers/user_defined.hpp>
#include <erslib/aescript/verifiers/vector.hpp>

#include <erslib/aescript/parsers/innate.hpp>
#include <erslib/aescript/parsers/user_defined.hpp>
#include <erslib/aescript/parsers/vector.hpp>


// Properties

#include <erslib/aescript/properties/exclusive_with.hpp>
#include <erslib/aescript/properties/inclusive_with.hpp>
#include <erslib/aescript/properties/one_of.hpp>
#include <erslib/aescript/properties/parse_into.hpp>
#include <erslib/aescript/properties/parse_into_with_index.hpp>
#include <erslib/aescript/properties/presence.hpp>
#include <erslib/aescript/properties/with_types.hpp>
