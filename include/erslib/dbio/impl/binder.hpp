#pragma once

// std
#include <functional>
#include <string>
#include <type_traits>

// pqxx
#include <pqxx/params>


namespace dbio::internal {
    using binder_t = std::function<void(pqxx::params&)>;

    template<typename T>
    using owned_t = std::conditional_t<
        std::convertible_to<T, std::string_view>,
        std::string,
        std::conditional_t<
        std::convertible_to<T, pqxx::bytes_view>,
        pqxx::bytes,
        std::remove_cvref_t<T>
        >
    >;

    template<typename T>
    binder_t make_binder(T&& value) {
        return [stored = owned_t<T>(std::forward<T>(value))](pqxx::params& out) {
            out.append(stored);
        };
    }
}
