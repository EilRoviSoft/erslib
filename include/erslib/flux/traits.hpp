#pragma once

#include <erslib/flux/impl/traits.hpp>

#include <erslib/flux/traits/basic.hpp>
#include <erslib/flux/traits/bool.hpp>
#include <erslib/flux/traits/optional.hpp>
#include <erslib/flux/traits/range.hpp>
#include <erslib/flux/traits/scalar.hpp>
#include <erslib/flux/traits/string.hpp>
#include <erslib/flux/traits/time.hpp>
#include <erslib/flux/traits/uuid.hpp>
#include <erslib/flux/traits/vec2.hpp>
#include <erslib/flux/traits/vector.hpp>


// Exports

namespace flux {
    using impl::sql_type;
    using impl::sql_value;
}
