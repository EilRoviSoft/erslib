#pragma once

// std
#include <concepts>
#include <format>
#include <functional>
#include <utility>

// ers
#include <erslib/core/type/diagnostic.hpp>
#include <erslib/core/type/maybe.hpp>
#include <erslib/core/type/result.hpp>


namespace utl::impl {
    struct result_policy {
        template<typename T>
        using type = ers::Result<T>;


        template<typename T, std::invocable Describe>
        static type<T> fail(Describe&& describe) {
            return ers::make_error(std::invoke(std::forward<Describe>(describe)));
        }

        template<typename T, typename U>
        static type<T> propagate(type<U>&& inner) {
            return std::move(inner).error();
        }

        template<typename T, typename U, typename... Args>
        static type<T> propagate(type<U>&& inner, std::format_string<Args...> fmt, Args&&... args) {
            return std::move(inner).context(fmt, std::forward<Args>(args)...).error();
        }
    };

    struct maybe_policy {
        template<typename T>
        using type = ers::maybe<T>;


        template<typename T, std::invocable Describe>
        static type<T> fail(Describe&&) {
            return ers::none;
        }

        template<typename T, typename U>
        static type<T> propagate(type<U>&&) {
            return ers::none;
        }

        template<typename T, typename U, typename... Args>
        static type<T> propagate(type<U>&&, std::format_string<Args...>, Args&&...) {
            return ers::none;
        }
    };


    template<typename Policy, typename T>
    using read_t = typename Policy::template type<T>;
}
