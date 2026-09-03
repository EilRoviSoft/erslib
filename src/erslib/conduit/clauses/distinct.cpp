#include "erslib/conduit/clauses/dinstinct.hpp"

// ers
#include <erslib/conduit/clauses/raw.hpp>
#include <erslib/conduit/slots/dinstinct.hpp>


conduit::impl::Clause conduit::impl::clauses::distinct() {
    return raw(&slots::distinct, " DISTINCT");
}
