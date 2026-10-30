#pragma once

#ifdef _HAS_BOOST_ASIO


// std
#include <string_view>

// ers
#include <erslib/core/type/result.hpp>


// Fwd -------------------------------------------------------------------------

namespace ers::impl {
    class AsyncRegistry;
}


// Implementation --------------------------------------------------------------

namespace ers::impl {
    class IService {
    public:
        // Member functions

        virtual ~IService() = default;


        // Accessors

        virtual std::string_view name() const = 0;


        // Modifiers

        virtual Status start() { return ok; }
        virtual void stop() {}
    };
}


// Exports ---------------------------------------------------------------------

namespace ers {
    using impl::IService;
}


#endif
