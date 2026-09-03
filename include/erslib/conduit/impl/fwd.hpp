#pragma once

// std
#include <memory>


namespace conduit::impl {
    class IClause;

    using Clause = std::polymorphic<IClause>;
}
