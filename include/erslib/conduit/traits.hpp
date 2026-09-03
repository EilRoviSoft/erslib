#pragma once

#include <erslib/conduit/impl/traits.hpp>

#include <erslib/conduit/traits/basic.hpp>
#include <erslib/conduit/traits/bool.hpp>
#include <erslib/conduit/traits/optional.hpp>
#include <erslib/conduit/traits/range.hpp>
#include <erslib/conduit/traits/scalar.hpp>
#include <erslib/conduit/traits/string.hpp>
#include <erslib/conduit/traits/vec2.hpp>


// Exports

namespace conduit {
    using impl::sql_type;
    using impl::sql_value;
}
