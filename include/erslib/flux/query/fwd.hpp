#pragma once

// std
#include <memory>


namespace flux::impl {
    class IClause;

    using Clause = std::polymorphic<IClause>;
}
