#pragma once

// std
#include <meta>
#include <type_traits>

// ers
#include <erslib/dbio/reflect/schema.hpp>
#include <erslib/dbio/sql/kind.hpp>


namespace dbio::impl::sql {
    template<typename T, typename Kind, typename Role>
    struct column_policy {
        template<std::meta::info M>
        static consteval bool includes() {
            if constexpr (std::is_same_v<Role, role::Read>) {
                if constexpr (std::is_same_v<Kind, kind::Insert> || std::is_same_v<Kind, kind::Save>)
                    return is_identity_column<T, M>();
                else if constexpr (std::is_same_v<Kind, kind::Select> || std::is_same_v<Kind, kind::LoadByPk>)
                    return true;
                else
                    return false;
            }
            else if constexpr (std::is_same_v<Role, role::Write>) {
                if constexpr (std::is_same_v<Kind, kind::Insert> || std::is_same_v<Kind, kind::Save>)
                    return !is_identity_column<T, M>();
                else if constexpr (std::is_same_v<Kind, kind::Update>)
                    return !is_identity_column<T, M>() && !is_pk_column<T, M>();
                else
                    return false;
            }
            else {
                if constexpr (std::is_same_v<Kind, kind::Save>)
                    return !is_identity_column<T, M>()
                        && !is_pk_column<T, M>()
                        && !is_conflict_column<T, M>();
                else if constexpr (std::is_same_v<Kind, kind::Select>)
                    return false;
                else
                    return is_pk_column<T, M>();
            }
        }
    };
}
