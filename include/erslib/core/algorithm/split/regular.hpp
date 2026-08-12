#pragma once

// ers
#include <erslib/core/algorithm/split/base.hpp>

// export
#include <erslib/export.hpp>


namespace ers::impl::algo {
    extern template class TSplitIterator<class RegularSplitIterator>;

    class ERSLIB_EXPORT RegularSplitIterator : public TSplitIterator<RegularSplitIterator> {
        friend class TSplitIterator;


    public:
        RegularSplitIterator() = default;
        explicit RegularSplitIterator(const SplitProcessor<RegularSplitIterator>& parent, size_t offset);


    private:
        void _advance();
    };
}
