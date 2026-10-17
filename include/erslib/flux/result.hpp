#pragma once

// Includes --------------------------------------------------------------------

#include <erslib/flux/result/query_result.hpp>
#include <erslib/flux/result/row.hpp>
#include <erslib/flux/result/row_generator.hpp>


// Exports ---------------------------------------------------------------------

namespace flux {
    using impl::QueryResult;
    using impl::ReadableRow;
    using impl::row_reader;
    using impl::RowGenerator;
}

namespace flux {
    using impl::for_each_row;
    using impl::read_one;
    using impl::read_at_most_one;
}
