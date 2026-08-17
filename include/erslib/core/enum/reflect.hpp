#pragma once

#ifndef ERSLIB_HAS_REFLECTION
#  error "erslib/core/enum/reflect requires C++26 static reflection (configure with ERSLIB_ENABLE_REFLECTION=ON)"
#endif


// std
#include <meta>
#include <array>
#include <string_view>
#include <type_traits>
#include <utility>

// ers
#include <erslib/core/meta.hpp>
#include <erslib/core/type/optional.hpp>
#include <erslib/core/convert/string.hpp>
#include <erslib/core/enum/fwd.hpp>
#include <erslib/core/enum/case_styles.hpp>
#include <erslib/core/enum/traits/enumerator_style.hpp>
#include <erslib/core/enum/traits/resolved_target_case.hpp>


// Per-enumerator name resolution

namespace ers::impl::enum_utils {
    template<typename E, E V>
    consteval std::string_view resolved_name() {
        using style = enumerator_style<E, V>;

        if constexpr (style::as_is) {
            return std::meta::identifier_of(meta::enumerator_of<E, V>());
        } else if constexpr (style::custom) {
            return std::define_static_string(
                meta::get_template_attribute_value<meta::enumerator_of<E, V>(), custom_t>().view()
            );
        } else {
            using source = case_style_traits<typename style::source_case>;
            using target = case_style_traits<typename resolved_target_case<E, V>::type>;

            auto raw = std::meta::identifier_of(meta::enumerator_of<E, V>());
            return std::define_static_string(target::combine(source::split(raw)));
        }
    }
}


// Name table

namespace ers::impl::enum_utils {
    template<typename E>
    consteval size_t enumerator_count() {
        return std::define_static_array(std::meta::enumerators_of(^^E)).size();
    }

    template<typename E>
    consteval auto build_name_table() {
        std::array<std::pair<E, std::string_view>, enumerator_count<E>()> table {};

        size_t i = 0;
        template for (constexpr auto e : std::define_static_array(std::meta::enumerators_of(^^E))) {
            constexpr E value = std::meta::extract<E>(e);
            table[i] = { value, resolved_name<E, value>() };
            ++i;
        }

        return table;
    }

    template<typename E>
    struct name_table {
        static constexpr auto value = build_name_table<E>();
    };
}


// Public conversion API

namespace ers::impl::enum_utils {
    template<typename E>
    constexpr std::string_view to_string(E value) {
        for (const auto& [v, name] : name_table<E>::value) {
            if (v == value)
                return name;
        }

        return {};
    }

    template<typename E>
    constexpr optional<E> from_string(std::string_view name) {
        for (const auto& [v, n] : name_table<E>::value) {
            if (n == name)
                return v;
        }

        return nullopt;
    }
}


// ers::convert integration

template<typename E>
    requires std::is_enum_v<E>
struct ers::convert::to_string_backend<E> {
    constexpr std::string_view constexpr_value(const E& value) const noexcept {
        return impl::enum_utils::to_string(value);
    }
};

template<typename E>
    requires std::is_enum_v<E>
struct ers::convert::from_string_backend<E> {
    Result<E> runtime_value(std::string_view source) const {
        if (auto value = ers::impl::enum_utils::from_string<E>(source))
            return *value;

        return make_error("Can't convert string \"{}\" to type [T = {}]",
            source, meta::type_name_v<E>);
    }
};
