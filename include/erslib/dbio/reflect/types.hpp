#pragma once

// std
#include <array>
#include <concepts>
#include <string_view>

// ers
#include <erslib/core/type/fixed_string.hpp>
#include <erslib/core/type/general.hpp>


// Entry kinds

namespace dbio::impl::reflect::kinds {
    struct table {};
    struct query {};

    struct pk {};
    struct unique {};
    struct fk {};

    struct def {};
    struct identity {};
    struct column {};
}

namespace dbio::impl::reflect {
    template<typename T, typename Kind>
    concept EntryOf = requires { typename T::kind; }
        && std::same_as<typename T::kind, Kind>;
}


// Declaration tags

namespace dbio::impl::reflect {
    template<ers::fixed_string Name>
    struct Table {
        using kind = kinds::table;
        static constexpr std::string_view name = Name.to_sv();
    };

    template<ers::fixed_string Name>
    struct Query {
        using kind = kinds::query;
        static constexpr std::string_view name = Name.to_sv();
    };
}


// Constraints

namespace dbio::impl::reflect {
    template<ers::fixed_string... Fields>
    struct Pk {
        using kind = kinds::pk;
        static constexpr std::array<std::string_view, sizeof...(Fields)> names = { Fields.to_sv()... };
    };

    template<ers::fixed_string... Fields>
    struct Unique {
        using kind = kinds::unique;
        static constexpr std::array<std::string_view, sizeof...(Fields)> names = { Fields.to_sv()... };
    };


    enum class action_on_delete : u8 {
        none,
        cascade,
        set_null,
        set_default,
        restrict,
    };

    template<
        ers::fixed_string Field,
        ers::fixed_string RefTable,
        ers::fixed_string RefColumn,
        action_on_delete OnDelete = action_on_delete::none
    >
    struct Fk {
        using kind = kinds::fk;
        static constexpr std::string_view field = Field.to_sv();
        static constexpr std::string_view ref_table = RefTable.to_sv();
        static constexpr std::string_view ref_column = RefColumn.to_sv();
        static constexpr action_on_delete on_delete = OnDelete;
    };
}


// Column metadata

namespace dbio::impl::reflect {
    template<ers::fixed_string Field, auto Value>
    struct Default {
        using kind = kinds::def;
        static constexpr std::string_view field = Field.to_sv();
        static constexpr auto value = Value;
    };

    template<ers::fixed_string Field>
    struct Identity {
        using kind = kinds::identity;
        static constexpr std::string_view field = Field.to_sv();
    };
}


// Annotations utility

namespace dbio::impl::reflect {
    template<ers::fixed_string Name = "", ers::fixed_string Type = "">
    struct column_t {
        using kind = kinds::column;
        static constexpr std::string_view name = Name.to_sv();
        static constexpr std::string_view type = Type.to_sv();
    };

    template<ers::fixed_string Name = "", ers::fixed_string Type = "">
    inline constexpr column_t<Name, Type> Column {};


    struct skip_t {};

    inline constexpr skip_t Skip {};
}
