#pragma once

// std
#include <cstdint>
#include <string_view>

// ers
#include <erslib/core/type/fixed_string.hpp>


// Column annotations

namespace dbio::impl {
    template<ers::fixed_string Value>
    struct TextTag {
        static constexpr std::string_view value = Value.to_sv();
    };


    // Implies identity for a sole integral key.
    struct Pk {};

    enum class OnDelete : uint8_t {
        NoAction, Cascade, SetNull, Restrict
    };

    template<typename Ref, ers::fixed_string Column, OnDelete Action = OnDelete::NoAction>
    struct Fk {
        using target = Ref;

        static constexpr std::string_view column = Column.to_sv();
        static constexpr OnDelete on_delete = Action;
    };


    // Overrides the sql_type_traits mapping.
    template<ers::fixed_string Type>
    struct Column : TextTag<Type> {};

    // Defaults to the member's identifier.
    template<ers::fixed_string Value>
    struct Name : TextTag<Value> {};

    template<ers::fixed_string Value>
    struct Default : TextTag<Value> {};


    // Columns are NOT NULL unless the type is nullable or this is present.
    struct Nullable {};

    struct Identity {};

    // In the definition, not in the table.
    struct Skip {};
}
