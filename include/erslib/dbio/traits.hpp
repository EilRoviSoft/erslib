#pragma once

#include <erslib/dbio/impl/traits.hpp>

#include <erslib/dbio/traits/basic.hpp>
#include <erslib/dbio/traits/bool.hpp>
#include <erslib/dbio/traits/optional.hpp>
#include <erslib/dbio/traits/range.hpp>
#include <erslib/dbio/traits/scalar.hpp>
#include <erslib/dbio/traits/string.hpp>
#include <erslib/dbio/traits/vec2.hpp>


// Exports

namespace dbio {
    using impl::sql_type;
    using impl::sql_value;
}
