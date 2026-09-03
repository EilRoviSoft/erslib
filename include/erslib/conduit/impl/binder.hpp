#pragma once

// std
#include <concepts>
#include <functional>
#include <string>
#include <type_traits>

// pqxx
#include <pqxx/params>


namespace conduit::impl {
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
}
