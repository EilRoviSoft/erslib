#pragma once

// ers
#include <erslib/core/meta/impl/funcsig.hpp>
#include <erslib/core/util/array.hpp>


namespace ers::internal {
    template<typename T>
    constexpr auto type_name_array() {
        constexpr auto name = funcsig<T>();
        return util::elems_as_array<std::string_view>(name, std::make_index_sequence<name.size()> {});
    }
}
