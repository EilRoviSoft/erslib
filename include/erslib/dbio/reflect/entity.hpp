#pragma once

// std
#include <meta>
#include <vector>

// ers
#include <erslib/dbio/reflect/traits.hpp>


namespace dbio::impl {
    // Columns only; metadata members never become storage.
    template<typename T>
    consteval std::vector<std::meta::info> entity_specs() {
        std::vector<std::meta::info> out;

        template for (constexpr auto m : std::define_static_array(columns<T>())) {
            out.emplace_back(std::meta::data_member_spec(
                std::meta::type_of(m),
                { .name = column_name<m>() })
            );
        }

        return out;
    }
}


// Exports

namespace dbio::reflect {
    using impl::entity_specs;
}


// Declares the entity's members from Definition<Type>.
#define ERS_DBIO_ENTITY(Type)                                                      \
    consteval {                                                                    \
        std::meta::define_aggregate(^^Type, dbio::reflect::entity_specs<Type>());  \
    }                                                                              \
    static_assert(dbio::reflect::Entity<Type>)
