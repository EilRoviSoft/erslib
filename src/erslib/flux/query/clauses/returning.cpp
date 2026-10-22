#include "erslib/flux/query/clauses/returning.hpp"

// ers
#include <erslib/flux/query/clauses/raw.hpp>


flux::impl::Clause flux::impl::clauses::returning_all() {
    return raw(&slots::returning, "*");
}
