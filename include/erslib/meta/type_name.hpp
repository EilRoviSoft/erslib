#pragma once

// std
#include <string_view>
#include <type_traits>
#include <utility>

// ers
#include <erslib/util/array.hpp>

namespace ers::meta::_impl {
    template<typename T>
    constexpr auto type_name_array() {
#ifdef __clang__
        constexpr auto prefix = std::string_view { "[T = " };
        constexpr auto suffix = std::string_view { "]" };
        constexpr auto function = std::string_view { __PRETTY_FUNCTION__ };
#elifdef __GNUC__
        constexpr auto prefix = std::string_view { "with T = " };
        constexpr auto suffix = std::string_view { "]" };
        constexpr auto function = std::string_view { __PRETTY_FUNCTION__ };
#elif defined(_MSC_VER)
        constexpr auto prefix = std::string_view { "type_name_array<" };
        constexpr auto suffix = std::string_view { ">(void)" };
        constexpr auto function = std::string_view { __FUNCSIG__ };
#else
#error Unsupported compiler
#endif

        constexpr auto start = function.find(prefix) + prefix.size();
        constexpr auto end = function.rfind(suffix);

        static_assert(start < end);

        constexpr auto name = function.substr(start, end - start);
        return util::elems_as_array<std::string_view>(name, std::make_index_sequence<name.size()> {});
    }

    template<typename T>
    struct type_name_holder {
        static constexpr auto value = type_name_array<T>();
    };
}

namespace ers::meta {
    template<typename T>
    struct type_name {
        constexpr std::string_view value = { _impl::type_name_holder<T>::value };
    };

    template<typename T>
    constexpr std::string_view type_name_v = type_name<T>::value;
}
