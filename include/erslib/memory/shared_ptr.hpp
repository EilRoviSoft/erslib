#pragma once

// boost
#include <boost/smart_ptr/atomic_shared_ptr.hpp>
#include <boost/smart_ptr/make_shared.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>


// Alias declaration

namespace ers {
    template<typename T>
    using shared_ptr = boost::shared_ptr<T>;

    template<typename T>
    using atomic_shared_ptr = boost::atomic_shared_ptr<T>;
}


// Function inclusion

namespace ers {
    using boost::make_shared;
}
