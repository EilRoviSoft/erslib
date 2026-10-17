#pragma once

// Includes --------------------------------------------------------------------

#include <erslib/flux/connection/database.hpp>
#include <erslib/flux/connection/options.hpp>
#include <erslib/flux/connection/scope.hpp>
#include <erslib/flux/connection/try_scope.hpp>


// Exports ---------------------------------------------------------------------

namespace flux {
    using impl::Database;
    using impl::db_options_t;
    using impl::pool_options_t;
}

namespace flux {
    using impl::ConnectionSource;
    using impl::with_conn;
    using impl::with_tx;
    using impl::with_query;
}

namespace flux {
    using impl::TryQueryResult;
    using impl::try_invoke;
    using impl::try_with_conn;
    using impl::try_with_tx;
    using impl::try_with_query;
}
