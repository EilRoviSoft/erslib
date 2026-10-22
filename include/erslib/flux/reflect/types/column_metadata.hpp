#pragma once

// std
#include <array>
#include <string>
#include <string_view>

// ers
#include <erslib/core/type/fixed_string.hpp>
#include <erslib/flux/reflect/types/entry.hpp>
#include <erslib/flux/reflect/types/sql.hpp>


namespace flux::impl::reflect {
    // Default

    template<ers::fixed_string Field, auto Value>
    struct Default {
        using kind = kinds::EDefault;
    };

    template<ers::fixed_string Field, auto Value>
    struct entry_traits<Default<Field, Value>> : entry_defaults {
        static constexpr std::string_view field = Field.to_sv();
        static constexpr std::array<std::string_view, 1> columns = { field };

        static consteval std::string column_ddl() {
            return " DEFAULT " + sql_literal<Value>();
        }
    };


    // ReadOnly

    template<ers::fixed_string Field>
    struct ReadOnly {
        using kind = kinds::EReadOnly;
    };

    template<ers::fixed_string Field>
    struct entry_traits<ReadOnly<Field>> : entry_defaults {
        static constexpr std::string_view field = Field.to_sv();
        static constexpr std::array<std::string_view, 1> columns = { field };
    };


    // SoftDelete

    template<ers::fixed_string Field>
    struct SoftDelete {
        using kind = kinds::ESoftDelete;
    };

    template<ers::fixed_string Field>
    struct entry_traits<SoftDelete<Field>> : entry_defaults {
        static constexpr std::string_view field = Field.to_sv();
        static constexpr std::array<std::string_view, 1> columns = { field };
    };


    // Identity

    template<ers::fixed_string Field>
    struct Identity {
        using kind = kinds::EIdentity;
    };

    template<ers::fixed_string Field>
    struct entry_traits<Identity<Field>> : entry_defaults {
        static constexpr std::string_view field = Field.to_sv();
        static constexpr std::array<std::string_view, 1> columns = { field };
    };
}
