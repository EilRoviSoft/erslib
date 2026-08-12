#pragma once


// Includes

#include <erslib/core/memory/any.hpp>
#include <erslib/core/memory/deleter.hpp>
#include <erslib/core/memory/holder.hpp>
#include <erslib/core/memory/shared_ptr.hpp>


// Exports

namespace ers {
    using impl::TAny;
    using impl::Any;

    using impl::deleter;

    using impl::holder_ptr;
    using impl::make_holder;
    using impl::make_polymorphic_holder;

    using impl::shared_ptr;
    using impl::atomic_shared_ptr;
    using impl::make_shared;
    using impl::static_pointer_cast;
    using impl::dynamic_pointer_cast;
    using impl::const_pointer_cast;
    using impl::reinterpret_pointer_cast;
}
