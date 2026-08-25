#include "erslib/dbio/clauses/dinstinct.hpp"

// ers
#include <erslib/dbio/clauses/raw.hpp>
#include <erslib/dbio/slots/dinstinct.hpp>


inline dbio::impl::Clause dbio::impl::clauses::distinct() {
    return raw(&slots::distinct, " DISTINCT");
}
