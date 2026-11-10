#pragma once

#ifndef ERSLIB_HAS_REFLECTION
#  error "erslib/core/enum/generate requires C++26 static reflection (configure with ERSLIB_ENABLE_REFLECTION=ON)"
#endif

// std
#include <algorithm>
#include <concepts>
#include <meta>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

// frozen
#include <frozen/string.h>
#include <frozen/unordered_set.h>

// ers
#include <erslib/core/stdlib/ctype.hpp>


// Policy ----------------------------------------------------------------------

namespace ers::impl::enum_gen {
    template<typename T>
    concept Policy = requires(typename T::enum_type value, std::string_view identifier) {
        { T::is_enabled(value, identifier) } -> std::convertible_to<bool>;
        { T::get_name(value, identifier) } -> std::convertible_to<std::string>;
    } && std::is_enum_v<typename T::enum_type>;
}


// Details ---------------------------------------------------------------------

namespace ers::impl::enum_gen {
    consteval bool is_identifier(std::string_view name) {
        if (name.empty() || is_digit(name[0]))
            return false;

        return std::ranges::all_of(name.substr(1), [](char c) {
            return is_alnum(c) || c == '_';
        });
    }

    consteval bool is_keyword(std::string_view name) {
        static constexpr auto keywords = frozen::make_unordered_set<frozen::string>({
            "break", "case", "catch", "class", "const", "continue", "default", "delete", "do", "else",
            "enum", "explicit", "export", "extern", "false", "for", "friend", "goto", "if", "inline",
            "namespace", "new", "operator", "private", "protected", "public", "register", "return",
            "static", "struct", "switch", "template", "this", "throw", "true", "try", "typedef",
            "typename", "union", "using", "virtual", "void", "volatile", "while",
        });

        return keywords.contains(frozen::string(name));
    }

    consteval void fail(const char* message) {
        throw message;
    }
}

namespace ers::impl::enum_gen {
    template<typename T>
    consteval void define_table(std::meta::info type) {
        using E = typename T::enum_type;

        std::vector<std::meta::info> specs;
        std::vector<std::string> taken;

        for (auto e : std::meta::enumerators_of(^^E)) {
            auto value = std::meta::extract<E>(e);
            auto id = std::meta::identifier_of(e);

            if (!T::is_enabled(value, id))
                continue;


            std::string name = T::get_name(value, id);

            if (!is_identifier(name))
                fail("enum_table: a generated name is not an identifier");

            if (is_keyword(name))
                fail("enum_table: a generated name is a keyword, exclude the enumerator or rename it");

            if (std::ranges::contains(taken, name))
                fail("enum_table: two enumerators have the same name");


            taken.push_back(name);

            specs.push_back(std::meta::data_member_spec(
                std::meta::substitute(^^T::template entry),
                {
                    .name      = std::meta::reflect_constant(value),
                    .alignment = {
                        .name              = std::define_static_string(name),
                        .no_unique_address = true,
                    }
                }
            ));
        }


        if (specs.empty())
            fail("enum_table: the policy leaves no enumerators");

        std::meta::define_aggregate(type, specs);
    }
}


// Implementation --------------------------------------------------------------

namespace ers::impl {
    template<enum_gen::Policy T>
    struct EnumTable {
        struct type;

        consteval { enum_gen::define_table<T>(^^type); }
    };

    template<enum_gen::Policy T>
    using enum_table_t = typename EnumTable<T>::type;

    template<enum_gen::Policy T>
    inline constexpr enum_table_t<T> enum_table {};
}


// Exports ---------------------------------------------------------------------

namespace ers {
    using impl::enum_table_t;
    using impl::enum_table;
}
