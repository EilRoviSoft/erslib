#pragma once


// Constant

#include <erslib/dbio/impl/constant.hpp>

namespace dbio {
    using impl::undefined_id;
}


// Database

#include <erslib/dbio/impl/database.hpp>

namespace dbio {
    using impl::Database;
}


// Generator

#include <erslib/dbio/impl/generator.hpp>

namespace dbio {
    using impl::ReadableRow;

    using impl::row_reader;
    using impl::RowGenerator;
}

namespace dbio {
    using impl::for_each_row;
    using impl::read_one;
    using impl::read_at_most_one;
}


// Options

#include <erslib/dbio/impl/options.hpp>

namespace dbio {
    using impl::db_options_t;
    using impl::pool_options_t;
}


// QueryStore

#include <erslib/dbio/impl/query_store.hpp>

namespace dbio {
    using impl::QueryStore;
    using impl::queries;
}
