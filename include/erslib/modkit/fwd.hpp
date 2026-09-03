#pragma once

// ers
#include <erslib/core/fwd.hpp>
#include <erslib/core/memory.hpp>


namespace ers::modkit::impl {
    using Object = Any;
}


// Exports

namespace ers::modkit {
    using impl::Object;
}
