#pragma once

// std
#include <memory>


namespace dbio::impl {
    class IClause;

    using ClausePtr = std::polymorphic<IClause>;
}
