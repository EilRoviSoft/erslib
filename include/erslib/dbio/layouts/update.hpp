#pragma once

// std
#include <string>

// ers
#include <erslib/dbio/impl/query_builder.hpp>

// export
#include <erslib/export.hpp>


namespace dbio::impl {
    namespace slots {
        inline constexpr Slot update {
            .name  = "dbio.update",
            .arity = Arity::Single,
        };
    }


    namespace layouts {
        ERSLIB_EXPORT QueryBuilder update(std::string table);
    }
}
