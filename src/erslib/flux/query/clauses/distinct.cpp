#include "erslib/flux/query/clauses/distinct.hpp"

// ers
#include <erslib/flux/query/clauses/raw.hpp>
#include <erslib/flux/query/slots/distinct.hpp>


flux::impl::Clause flux::impl::clauses::distinct() {
    return raw(&slots::distinct, " DISTINCT");
}
