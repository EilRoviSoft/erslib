#pragma once

// std
#include <string>

// ers
#include <erslib/conduit/impl/query_builder.hpp>

// export
#include <erslib/export.hpp>


namespace conduit::impl {
    namespace slots {
        inline constexpr Slot delete_from = {
            .name  = "conduit.delete_from",
            .arity = EArity::Single,
        };
    }


    namespace layouts {
        QueryBuilder ERSLIB_EXPORT delete_from(std::string table);
    }
}
