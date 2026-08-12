#pragma once

// ers
#include <erslib/core/algorithm/split/base.hpp>

// export
#include <erslib/export.hpp>


namespace ers::impl::algo {
    class ERSLIB_EXPORT SmartSplitIterator : public TSplitIterator<SmartSplitIterator> {
        friend class TSplitIterator;


    public:
        SmartSplitIterator() = default;
        explicit SmartSplitIterator(const SplitProcessor<SmartSplitIterator>& parent, size_t offset);


    private:
        void _advance();
    };
}
