#pragma once


// Goal is to have something like this:
// { "energy-source", required() | expected_types<ElectricEnergySource, VoidEnergySource> },
// { "active-energy-usage", required() | expected_type<Energy> }
// { "icons", expected_type<vector<IconData>> | exclusive_with({ "icon", "icon-size" }) }
// { "icon", expected_type<FileName>() | exclusive_with("icons") }
// { "icon-size , expected_type<SpriteSizeType> | exclusive_with("icons") }


#include <aescript/impl/field.hpp>
#include <aescript/impl/layout.hpp>
#include <aescript/impl/parser.hpp>
#include <aescript/impl/verifier.hpp>

#include <aescript/verifiers/exclusive_with.hpp>
#include <aescript/verifiers/presence.hpp>
#include <aescript/verifiers/with_types.hpp>

#include <aescript/parsers/innate.hpp>
#include <aescript/parsers/into.hpp>
#include <aescript/parsers/user_defined.hpp>
