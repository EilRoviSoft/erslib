#pragma once

// std
#include <erslib/core/algorithm/split/regular.hpp>
#include <erslib/core/algorithm/split/smart.hpp>
#include <erslib/core/algorithm/split/base.hpp>


namespace ers {
    using RegularSplitter = impl::SplitProcessor<impl::RegularSplitIterator>;
    using SmartSplitter = impl::SplitProcessor<impl::SmartSplitIterator>;
}
