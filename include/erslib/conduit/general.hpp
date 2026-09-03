#pragma once


// Constant

#include <erslib/conduit/impl/constant.hpp>

namespace conduit {
    using impl::undefined_id;
}


// Database

#include <erslib/conduit/impl/database.hpp>

namespace conduit {
    using impl::Database;
}


// Generator

#include <erslib/conduit/impl/generator.hpp>

namespace conduit {
    using impl::ReadableRow;

    using impl::row_reader;
    using impl::RowGenerator;
}

namespace conduit {
    using impl::for_each_row;
    using impl::read_one;
    using impl::read_at_most_one;
}


// Options

#include <erslib/conduit/impl/options.hpp>

namespace conduit {
    using impl::db_options_t;
    using impl::pool_options_t;
}


// QueryStore

#include <erslib/conduit/impl/query_store.hpp>

namespace conduit {
    using impl::QueryStore;
    using impl::queries;
}
