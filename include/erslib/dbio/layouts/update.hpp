#pragma once

// std
#include <string>

// ers
#include <erslib/dbio/impl/query.hpp>

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
        ERSLIB_EXPORT Query update(std::string table);
    }
}
