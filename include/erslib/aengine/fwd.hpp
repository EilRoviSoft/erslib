#pragma once

// ers
#include <erslib/core/fwd.hpp>
#include <erslib/core/memory.hpp>


namespace aengine::impl {
    using Object = ers::Any;
}


// Exports

namespace aengine {
    using impl::Object;
}
