#pragma once

// std
#include <string>

// pqxx
#include <pqxx/dbtransaction>
#include <pqxx/params>

// ers
#include <erslib/conduit/impl/binder.hpp>

// export
#include <erslib/export.hpp>


namespace conduit::impl {
    struct ERSLIB_EXPORT Context {
        std::string query;
        pqxx::params params;
        pqxx::placeholders<> counter;


        // Member functions

        Context() = default;

        explicit Context(const pqxx::dbtransaction& tx);

        Context(Context&&) = default;
        Context& operator=(Context&&) = default;

        Context(const Context&) = delete;
        Context& operator=(const Context&) = delete;

        ~Context() = default;


        // Modifiers

        std::string bind(const binder_t& binder);
    };
}
