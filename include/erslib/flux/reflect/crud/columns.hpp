#pragma once

// std
#include <concepts>
#include <cstddef>
#include <meta>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

// ers
#include <erslib/flux/expr/expr.hpp>
#include <erslib/flux/reflect/schema.hpp>


// Implementation --------------------------------------------------------------

namespace flux::impl::crud {
    template<std::meta::info M>
    class TypedColumn : public ExprMethods {
    public:
        // Member types

        using value_type = reflect::member_type<M>;


        // Member functions

        constexpr TypedColumn() = default;

        constexpr explicit TypedColumn(std::string_view qualifier) :
            _qualifier(qualifier) {
        }


        // Accessors

        static constexpr std::string_view name() noexcept { return reflect::column_name<M>; }


        // Generators

        Expr to_expr() const {
            return _qualifier.empty()
                ? clauses::col(std::string(name()))
                : clauses::col(_qualifier, name());
        }


        template<typename U>
        static constexpr bool accepts = IsExpr<U> || std::convertible_to<U, value_type>;


    private:
        std::string_view _qualifier;
    };


    template<typename T>
    struct Columns {
        struct type;

        consteval {
            std::vector<std::meta::info> specs;

            for (const auto m : reflect::column_list<T>) {
                specs.emplace_back(std::meta::data_member_spec(
                    std::meta::substitute(^^TypedColumn, { std::meta::reflect_constant(m) }),
                    { .name = std::meta::identifier_of(m) }
                ));
            }

            std::meta::define_aggregate(^^type, specs);
        }
    };

    template<typename T>
    using columns_t = typename Columns<T>::type;


    template<typename T>
    constexpr columns_t<T> columns_as(std::string_view qualifier) {
        return [&]<std::size_t... I>(std::index_sequence<I...>) {
            return columns_t<T> { TypedColumn<reflect::column_list<T>[I]> { qualifier }... };
        }(std::make_index_sequence<reflect::column_list<T>.size()>());
    }

    template<typename T>
    inline constexpr columns_t<T> columns_of = columns_as<T>({});
}
