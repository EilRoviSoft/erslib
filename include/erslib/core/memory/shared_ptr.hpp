#pragma once

#ifndef __CLANG__


// std
#include <atomic>
#include <memory>


namespace ers::impl {
    using std::shared_ptr;
    template<typename T>
    using atomic_shared_ptr = std::atomic<std::shared_ptr<T>>;

    using std::enable_shared_from_this;
    using std::make_shared;

    using std::static_pointer_cast;
    using std::dynamic_pointer_cast;
    using std::const_pointer_cast;
    using std::reinterpret_pointer_cast;
}


#elifdef _HAS_BOOST_SMART_PTR


// boost
#include <boost/smart_ptr/atomic_shared_ptr.hpp>
#include <boost/smart_ptr/enable_shared_from_this.hpp>
#include <boost/smart_ptr/make_shared.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>


namespace ers::impl {
    using boost::shared_ptr;
    using boost::atomic_shared_ptr;

    using boost::enable_shared_from_this;
    using boost::make_shared;

    using boost::static_pointer_cast;
    using boost::dynamic_pointer_cast;
    using boost::const_pointer_cast;
    using boost::reinterpret_pointer_cast;
}


#else

#error "Clang compiler doesn't support atomic_shared_ptr, can't fallback to std implementation"

#endif
