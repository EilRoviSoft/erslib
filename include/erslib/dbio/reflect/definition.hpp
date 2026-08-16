#pragma once

// std
#include <array>
#include <cstddef>
#include <string_view>

// ers
#include <erslib/core/type/fixed_string.hpp>


namespace dbio {
    // Specialized per entity. Non-static members are columns; everything else is
    // metadata, and the table identity comes from the Table base.
    template<typename T>
    struct Definition;
}


// Table identity and constraints

namespace dbio::impl {
    template<ers::fixed_string Name>
    struct Table {
        static constexpr std::string_view table = Name.to_sv();
    };

    template<ers::fixed_string... Fields>
    struct Unique {
        static constexpr std::array<std::string_view, sizeof...(Fields)> fields { Fields.to_sv()... };
    };
}


// Statements

namespace dbio::impl::statement {
    struct SelectAll {};
    struct Save {};
    struct Update {};
    struct Delete {};

    template<ers::fixed_string... Fields>
    struct LoadBy {
        static constexpr std::array<std::string_view, sizeof...(Fields)> fields { Fields.to_sv()... };
    };

    template<ers::fixed_string... Fields>
    struct DeleteBy {
        static constexpr std::array<std::string_view, sizeof...(Fields)> fields { Fields.to_sv()... };
    };
}


// Exports

namespace dbio::reflect {
    using impl::Table;
    using impl::Unique;

    namespace statement = impl::statement;
}
