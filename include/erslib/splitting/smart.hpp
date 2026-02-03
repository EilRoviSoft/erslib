#pragma once

// ers
#include <erslib/shared.hpp>
#include <erslib/splitting/base.hpp>

namespace ers::splitting {
    class ERSLIB_API SmartIterator : public TIterator<SmartIterator> {
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
