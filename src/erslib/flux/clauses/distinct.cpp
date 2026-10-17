#include "erslib/flux/clauses/dinstinct.hpp"

// ers
#include <erslib/flux/clauses/raw.hpp>
#include <erslib/flux/slots/dinstinct.hpp>


flux::impl::Clause flux::impl::clauses::distinct() {
    return raw(&slots::distinct, " DISTINCT");
}
