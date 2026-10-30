#pragma once


// Includes --------------------------------------------------------------------

#include <erslib/core/algorithm/container.hpp>
#include <erslib/core/algorithm/hash.hpp>
#include <erslib/core/algorithm/args.hpp>
#include <erslib/core/algorithm/split/regular.hpp>
#include <erslib/core/algorithm/split/smart.hpp>


// Exports ---------------------------------------------------------------------

namespace ers {
    using impl::keys_sorted_by_value;
    using impl::insert_sorted;
    using impl::hash_combine;

    using impl::split_args;
    using impl::quote_arg;
    using impl::join_args;

    using RegularSplitter = impl::SplitProcessor<impl::RegularSplitIterator>;
    using SmartSplitter = impl::SplitProcessor<impl::SmartSplitIterator>;
}
