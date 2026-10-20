#pragma once


// Includes --------------------------------------------------------------------

#include <erslib/core/memory/any.hpp>
#include <erslib/core/memory/deleter.hpp>
#include <erslib/core/memory/holder.hpp>


// Exports ---------------------------------------------------------------------

namespace ers {
    using impl::TAny;
    using impl::Any;

    using impl::deleter;

    using impl::holder_ptr;
    using impl::make_holder;
}
