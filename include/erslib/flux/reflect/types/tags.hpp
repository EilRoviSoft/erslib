#pragma once

// std
#include <string_view>
#include <type_traits>

// ers
#include <erslib/core/type/fixed_string.hpp>
#include <erslib/flux/reflect/types/entry.hpp>


namespace flux::impl::reflect {
    template<ers::fixed_string Name>
    struct Schema {
        using kind = kinds::ESchema;
        static constexpr std::string_view name = Name.to_sv();

        static_assert(!Name.to_sv().empty(), "Schema<> needs a name.");
        static_assert(!Name.to_sv().contains('.'), "Schema<> name must not be qualified.");
    };

    struct NoSchema {
        using kind = kinds::ESchema;
        static constexpr std::string_view name = "";
    };

    template<typename T>
    concept SchemaType = EntryOf<T, kinds::ESchema>;


    template<ers::fixed_string Name, SchemaType S = NoSchema>
    struct Table {
        using kind = kinds::ETable;
        using schema = S;

        static constexpr std::string_view name = Name.to_sv();

        static_assert(std::is_same_v<S, NoSchema> || !Name.to_sv().contains('.'),
            "Table<> name must not be qualified when a schema is given, the schema type qualifies it.");
    };

    template<ers::fixed_string Name>
    struct Query {
        using kind = kinds::EQuery;
        static constexpr std::string_view name = Name.to_sv();
    };
}
