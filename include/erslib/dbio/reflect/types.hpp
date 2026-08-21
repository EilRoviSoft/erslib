#pragma once

// std
#include <array>
#include <meta>
#include <string_view>
#include <type_traits>

// ers
#include <erslib/core/type/fixed_string.hpp>


// Table

namespace dbio::impl {
    template<ers::fixed_string Name>
    struct Table {
        static constexpr auto table_name = Name;
    };


    template<typename T>
    struct is_table : std::false_type {};

    template<ers::fixed_string Name>
    struct is_table<Table<Name>> : std::true_type {};
}


// Metadata

namespace dbio::impl {
    template<ers::fixed_string... Fields>
    struct Pk {};


    template<typename T>
    struct is_pk : std::false_type {};

    template<ers::fixed_string... Fields>
    struct is_pk<Pk<Fields...>> : std::true_type {};


    template<typename T>
    struct pk_fields;

    template<ers::fixed_string... Fields>
    struct pk_fields<Pk<Fields...>> {
        static constexpr std::array<std::string_view, sizeof...(Fields)> names = {
            std::string_view(Fields.value, Fields.strlen())...
        };
    };
}


namespace dbio::impl {
    enum class action_on_delete {
        none,
        cascade,
        set_null,
        set_default,
        restrict,
    };


    template<
        ers::fixed_string Field,
        ers::fixed_string Table,
        ers::fixed_string Column,
        action_on_delete OnDelete = action_on_delete::none
    >
    struct Fk {};


    template<typename T>
    struct is_fk : std::false_type {};

    template<ers::fixed_string Field, ers::fixed_string Table,
        ers::fixed_string Column, action_on_delete OnDelete>
    struct is_fk<Fk<Field, Table, Column, OnDelete>> : std::true_type {};


    template<typename T>
    struct fk_info;

    template<ers::fixed_string Field, ers::fixed_string Table,
        ers::fixed_string Column, action_on_delete OnDelete>
    struct fk_info<Fk<Field, Table, Column, OnDelete>> {
        static constexpr auto field = std::string_view(Field.value, Field.strlen());
        static constexpr auto ref_table = std::string_view(Table.value, Table.strlen());
        static constexpr auto ref_column = std::string_view(Column.value, Column.strlen());
        static constexpr action_on_delete on_delete = OnDelete;
    };
}


namespace dbio::impl {
    template<ers::fixed_string... Fields>
    struct Unique {};


    template<typename T>
    struct is_unique : std::false_type {};

    template<ers::fixed_string... Fields>
    struct is_unique<Unique<Fields...>> : std::true_type {};


    template<typename T>
    struct unique_fields;

    template<ers::fixed_string... Fields>
    struct unique_fields<Unique<Fields...>> {
        static constexpr std::array<std::string_view, sizeof...(Fields)> names = {
            std::string_view(Fields.value, Fields.strlen())...
        };
    };
}


namespace dbio::impl {
    template<ers::fixed_string Field, auto Value>
    struct Default {};


    template<typename T>
    struct is_default : std::false_type {};

    template<ers::fixed_string Field, auto Value>
    struct is_default<Default<Field, Value>> : std::true_type {};


    template<typename T>
    struct default_info;

    template<ers::fixed_string Field, auto Value>
    struct default_info<Default<Field, Value>> {
        static constexpr std::string_view field = std::string_view(Field.value, Field.strlen());
        static constexpr auto value = Value;
    };
}


namespace dbio::impl {
    template<ers::fixed_string Field>
    struct Identity {};


    template<typename T>
    struct is_identity : std::false_type {};

    template<ers::fixed_string Field>
    struct is_identity<Identity<Field>> : std::true_type {};


    template<typename T>
    struct identity_info;

    template<ers::fixed_string Field>
    struct identity_info<Identity<Field>> {
        static constexpr std::string_view field = std::string_view(Field.value, Field.strlen());
    };
}


// Annotations utility

namespace dbio::impl {
    template<ers::fixed_string Name = "", ers::fixed_string Type = "">
    struct column_t {};


    template<typename T>
    struct is_column : std::false_type {};

    template<ers::fixed_string Name, ers::fixed_string Type>
    struct is_column<column_t<Name, Type>> : std::true_type {};


    template<typename T>
    struct column_info;

    template<ers::fixed_string Name, ers::fixed_string Type>
    struct column_info<column_t<Name, Type>> {
        static constexpr std::string_view name = std::string_view(Name.value, Name.strlen());
        static constexpr std::string_view type = std::string_view(Type.value, Type.strlen());
    };


    template<ers::fixed_string Name = "", ers::fixed_string Type = "">
    inline constexpr column_t<Name, Type> Column {};
}


namespace dbio::impl {
    struct skip_t {};


    template<typename T>
    struct is_skip : std::false_type {};

    template<>
    struct is_skip<skip_t> : std::true_type {};


    inline constexpr skip_t Skip {};
}
