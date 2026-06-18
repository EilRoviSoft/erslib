#pragma once


// Goal is to have something like this:
// { "energy-source", required() | expected_types<ElectricEnergySource, VoidEnergySource> },
// { "active-energy-usage", required() | expected_type<Energy> }
// { "icons", expected_type<vector<IconData>> | exclusive_with({ "icon", "icon-size" }) }
// { "icon", expected_type<FileName>() | exclusive_with("icons") }
// { "icon-size , expected_type<SpriteSizeType> | exclusive_with("icons") }


// In future this can be done with reflection:
// [[=reflect::Field() | presence(Required)]]
// std::string type;


// Main implementation

#include <aescript/impl/field.hpp>
#include <aescript/impl/layout.hpp>
#include <aescript/impl/parser.hpp>
#include <aescript/impl/verifier.hpp>

#include <aescript/impl/util.hpp>


// Utility

#include <aescript/verifiers/innate.hpp>
#include <aescript/verifiers/user_defined.hpp>
#include <aescript/verifiers/vector.hpp>

#include <aescript/parsers/innate.hpp>
#include <aescript/parsers/user_defined.hpp>
#include <aescript/parsers/vector.hpp>


// Properties

#include <aescript/verifiers/exclusive_with.hpp>
#include <aescript/verifiers/presence.hpp>
#include <aescript/verifiers/with_types.hpp>

#include <aescript/parsers/into.hpp>
