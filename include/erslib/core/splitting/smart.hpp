#pragma once

// ers
#include <erslib/core/splitting/base.hpp>

// export
#include <erslib/export.hpp>


namespace ers::impl::splitting {
    class ERSLIB_EXPORT SmartIterator : public TIterator<SmartIterator> {
        friend class TIterator;


    public:
        SmartIterator() = default;
        explicit SmartIterator(const Processor<SmartIterator>& parent, size_t offset);


    private:
        void _advance();
    };
}


namespace ers::impl {
    using SmartSplitter = splitting::Processor<splitting::SmartIterator>;
}

namespace ers {
    using impl::SmartSplitter;
}
