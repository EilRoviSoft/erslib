#pragma once

// std
#include <string>

// ers
#include <erslib/dbio/impl/query.hpp>

// export
#include <erslib/export.hpp>


namespace dbio::impl {
    namespace slots {
        inline constexpr Slot delete_from = {
            .name   = "dbio.delete_from",
            .prefix = "DELETE FROM ",
            .arity  = Arity::Single,
        };
    }


    namespace layouts {
        ERSLIB_EXPORT Query delete_from(std::string table);
    }
}
