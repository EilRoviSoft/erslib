#pragma once

// ers
#include <erslib/core/meta/impl/funcsig.hpp>
#include <erslib/core/util/fixed_string.hpp>


namespace ers::impl::meta {
    template<typename T>
    constexpr auto type_name_literal() {
        constexpr auto name = funcsig<T>();
        return util::elems_as_string_literal<std::string_view>(name, std::make_index_sequence<name.size()> {});
    }
}


namespace ers::impl::meta {
    template<typename T>
    struct type_literal {
        static constexpr auto value = type_name_literal<T>();
    };

    template<typename T>
    inline constexpr auto type_literal_v = type_literal<T>::value;
}
