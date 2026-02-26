#pragma once

// ers
#include <erslib/splitting/base.hpp>

// export
#include <erslib/export.hpp>


namespace ers::splitting {
    extern template class TIterator<class RegularIterator>;

    class ERSLIB_EXPORT RegularIterator : public TIterator<RegularIterator> {
        friend class TIterator;


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
