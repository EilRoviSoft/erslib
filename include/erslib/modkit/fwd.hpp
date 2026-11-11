#pragma once

// ers
#include <erslib/core/fwd.hpp>
#include <erslib/core/memory.hpp>


namespace ers::impl::modkit {
    using Object = Any;
}


// Exports ---------------------------------------------------------------------

namespace ers::modkit {
    using impl::modkit::Object;
}
