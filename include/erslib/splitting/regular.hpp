#pragma once

// ers
#include <erslib/splitting/base.hpp>

namespace ers::splitting {
    class RegularIterator : public BaseIterator<RegularIterator> {
        friend class BaseIterator;

    public:
        RegularIterator() = default;
        explicit RegularIterator(const Processor<RegularIterator>& parent, size_t offset);

    private:
        void _advance();
    };
}

namespace ers {
    using RegularSplitter = splitting::Processor<splitting::RegularIterator>;
}
