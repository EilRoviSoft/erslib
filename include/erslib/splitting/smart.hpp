#pragma once

// ers
#include <erslib/splitting/base.hpp>

namespace ers::splitting {
    class SmartIterator : public BaseIterator<SmartIterator> {
        friend class BaseIterator;

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
