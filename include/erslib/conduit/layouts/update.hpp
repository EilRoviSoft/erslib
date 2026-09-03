#pragma once

// std
#include <string>

// ers
#include <erslib/conduit/impl/query_builder.hpp>

// export
#include <erslib/export.hpp>


namespace conduit::impl {
    namespace slots {
        inline constexpr Slot update {
            .name  = "conduit.update",
            .arity = EArity::Single,
        };
    }


    namespace layouts {
        QueryBuilder ERSLIB_EXPORT update(std::string table);
    }
}
