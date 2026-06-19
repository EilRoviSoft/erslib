#pragma once

#ifdef _HAS_BOOST_SMART_PTR


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

    using boost::static_pointer_cast;
    using boost::dynamic_pointer_cast;
    using boost::const_pointer_cast;
    using boost::reinterpret_pointer_cast;
}


#elifdef !__CLANG__


// std
#include <atomic>
#include <memory>


// Alias declaration

namespace ers {
    template<typename T>
    using shared_ptr = std::shared_ptr<T>;

    template<typename T>
    using atomic_shared_ptr = std::atomic<std::shared_ptr<T>>;
}


// Function inclusion

namespace ers {
    using std::make_shared;

    using std::static_pointer_cast;
    using std::dynamic_pointer_cast;
    using std::const_pointer_cast;
    using std::reinterpret_pointer_cast;
}


#else

#error "Clang compiler doesn't support atomic_shared_ptr, can't fallback to std implementation"

#endif
