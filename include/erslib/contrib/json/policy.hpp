#pragma once

// std
#include <concepts>
#include <format>
#include <functional>
#include <type_traits>
#include <utility>

// ers
#include <erslib/core/type/diagnostic.hpp>
#include <erslib/core/type/maybe.hpp>
#include <erslib/core/type/result.hpp>


namespace utl::impl {
    template<typename T>
    struct maybe_result {
        using type = ers::maybe<T>;
    };

    template<>
    struct maybe_result<void> {
        using type = bool;
    };


    struct result_policy {
        template<typename T>
        using type = ers::Result<T>;


        static type<void> done() {
            return ers::ok;
        }

        template<typename T, std::invocable Describe>
        static type<T> fail(Describe&& describe) {
            return ers::make_error(std::invoke(std::forward<Describe>(describe)));
        }

        template<typename T, typename Inner>
        static type<T> propagate(Inner&& inner) {
            return std::move(inner).error();
        }

        template<typename T, typename Inner, typename... Args>
        static type<T> propagate(Inner&& inner, std::format_string<Args...> fmt, Args&&... args) {
            return std::move(inner).context(fmt, std::forward<Args>(args)...).error();
        }
    };

    struct maybe_policy {
        template<typename T>
        using type = typename maybe_result<T>::type;


        static type<void> done() {
            return true;
        }

        template<typename T, std::invocable Describe>
        static type<T> fail(Describe&&) {
            if constexpr (std::is_void_v<T>)
                return false;
            else
                return ers::none;
        }

        template<typename T, typename Inner>
        static type<T> propagate(Inner&&) {
            if constexpr (std::is_void_v<T>)
                return false;
            else
                return ers::none;
        }

        template<typename T, typename Inner, typename... Args>
        static type<T> propagate(Inner&&, std::format_string<Args...>, Args&&...) {
            if constexpr (std::is_void_v<T>)
                return false;
            else
                return ers::none;
        }
    };


    template<typename Policy, typename T>
    using read_t = typename Policy::template type<T>;

    template<typename Policy>
    using write_t = typename Policy::template type<void>;
}
