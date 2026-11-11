#pragma once

#include <erslib/flux/sql/value.hpp>

#include <erslib/flux/traits/basic.hpp>
#include <erslib/flux/traits/bool.hpp>
#include <erslib/flux/traits/maybe.hpp>
#include <erslib/flux/traits/range.hpp>
#include <erslib/flux/traits/scalar.hpp>
#include <erslib/flux/traits/string.hpp>
#include <erslib/flux/traits/time.hpp>
#include <erslib/flux/traits/tsvector.hpp>
#include <erslib/flux/traits/uuid.hpp>
#include <erslib/flux/traits/vec2.hpp>
#include <erslib/flux/traits/vector.hpp>

#include <erslib/flux/traits/named_tags.hpp>


// Exports ---------------------------------------------------------------------

namespace flux {
    using impl::sql_type;
    using impl::sql_value;

    using namespace impl::reflect::named_tags;
}
