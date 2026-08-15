#pragma once

// std
#include <memory>


namespace dbio::impl {
    class IClause;

    using Clause = std::polymorphic<IClause>;
}
