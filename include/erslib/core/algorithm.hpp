#pragma once


// Includes

#include <erslib/core/algorithm/container.hpp>
#include <erslib/core/algorithm/hash.hpp>
#include <erslib/core/algorithm/split/regular.hpp>
#include <erslib/core/algorithm/split/smart.hpp>


// Exports

namespace ers::algo {
    using impl::algo::keys_sorted_by_value;
    using impl::algo::combine;

    using RegularSplitter = impl::algo::SplitProcessor<impl::algo::RegularSplitIterator>;
    using SmartSplitter = impl::algo::SplitProcessor<impl::algo::SmartSplitIterator>;
}
