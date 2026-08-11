#pragma once

// std
#include <string>

// pqxx
#include <pqxx/dbtransaction>
#include <pqxx/params>

// ers
#include <erslib/dbio/impl/binder.hpp>

// export
#include <erslib/export.hpp>


namespace dbio {
    struct ERSLIB_EXPORT build_context_t {
        std::string query;
        pqxx::params params;
        pqxx::placeholders<> counter;


        // Member functions

        build_context_t() = default;

        explicit build_context_t(const pqxx::dbtransaction& tx);

        build_context_t(build_context_t&&) = default;
        build_context_t& operator=(build_context_t&&) = default;

        build_context_t(const build_context_t&) = delete;
        build_context_t& operator=(const build_context_t&) = delete;

        ~build_context_t() = default;


        // Modifiers

        std::string bind(const internal::binder_t& binder);

        std::string bind_null();
    };
}
