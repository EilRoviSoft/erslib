#pragma once

// ers
#include <erslib/splitting/base.hpp>

// export
#include <erslib/export.hpp>


namespace ers::splitting {
    class ERSLIB_EXPORT SmartIterator : public TIterator<SmartIterator> {
        friend class TIterator;

    public:
        SmartIterator() = default;
        explicit SmartIterator(const Processor<SmartIterator>& parent, size_t offset);

    private:
        void _advance();
    };
}


namespace ers {
    using SmartSplitter = splitting::Processor<splitting::SmartIterator>;
}
