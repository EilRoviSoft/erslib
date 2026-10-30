#pragma once

#ifdef _HAS_BOOST_ASIO


// Fwd -------------------------------------------------------------------------

namespace ers::impl {
    class AsyncRegistry;
}


// Implementation --------------------------------------------------------------

namespace ers::impl {
    class IWorker {
    public:
        // Member functions

        virtual ~IWorker() = default;


        // Modifiers

        virtual void declare(AsyncRegistry& registry) = 0;
    };
}


// Exports ---------------------------------------------------------------------

namespace ers {
    using impl::IWorker;
}


#endif
