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


// Eval

#include <erslib/dbio/impl/eval.hpp>

namespace dbio {
    namespace tag = impl::tag;
    using impl::tx_tag;
}


// Formatter

#include <erslib/dbio/impl/formatter.hpp>

namespace dbio {
    using impl::Mode;
    using impl::bytes_to_string;
}


// Generator

#include <erslib/dbio/impl/generator.hpp>

namespace dbio {
    using impl::ValidRow;
    using impl::ReadableRow;

    using impl::row_reader;
    using impl::RowGenerator;
}

namespace dbio {
    using impl::for_each_row;
    using impl::read_one;
    using impl::read_at_most_one;
}


// QueryStore

#include <erslib/dbio/impl/query_store.hpp>

namespace dbio {
    using impl::QueryStore;
    using impl::queries;
}
