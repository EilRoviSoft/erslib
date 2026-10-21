#pragma once

// std
#include <string>
#include <unordered_map>

// pqxx
#include <pqxx/dbtransaction>
#include <pqxx/params>

// ers
#include <erslib/flux/sql/binder.hpp>

// export
#include <erslib/export.hpp>


namespace flux::impl {
    struct ERSLIB_EXPORT Context {
        std::string query;
        pqxx::params params;
        pqxx::placeholders<> counter;
        std::unordered_map<const void*, std::string> bound;


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

        // The same key gets the same placeholder within one statement.
        std::string bind_once(const void* key, const binder_t& binder);
    };
}
