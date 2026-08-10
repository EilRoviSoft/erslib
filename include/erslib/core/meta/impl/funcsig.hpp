#pragma once

#ifdef ERSLIB_HAS_REFLECTION


// std
#include <meta>
#include <string_view>


namespace ers::internal {
    template<typename T>
    constexpr auto funcsig() {
        constexpr auto name = std::meta::display_string_of(^^T);
        constexpr auto* storage = std::define_static_string(name);
        return std::string_view { storage };
    }
}


#else


// std
#include <string_view>


namespace ers::internal {
    template<typename T>
    constexpr auto funcsig() {
#ifdef __clang__
        constexpr auto prefix = std::string_view { "[T = " };
        constexpr auto suffix = std::string_view { "]" };
        constexpr auto function = std::string_view { __PRETTY_FUNCTION__ };
#elifdef __GNUC__
        constexpr auto prefix = std::string_view { "with T = " };
        constexpr auto suffix = std::string_view { "]" };
        constexpr auto function = std::string_view { __PRETTY_FUNCTION__ };
#elifdef _MSC_VER
        constexpr auto prefix = std::string_view { "funcsig<" };
        constexpr auto suffix = std::string_view { ">(void)" };
        constexpr auto function = std::string_view { __FUNCSIG__ };
#else
#error Unsupported compiler
#endif


        constexpr auto start = function.find(prefix) + prefix.size();
        constexpr auto end = function.rfind(suffix);


        static_assert(start < end);

        return function.substr(start, end - start);
    }
}


#endif
