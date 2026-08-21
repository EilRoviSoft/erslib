#pragma once

// std
#include <meta>

// pqxx
#include <pqxx/params>

// ers
#include <erslib/dbio/reflect/schema.hpp>
#include <erslib/dbio/reflect/value.hpp>


namespace dbio::impl::reflect {
    template<typename Input>
    void bind_params(pqxx::params& out, const Input& in) {
        template for (constexpr auto m : std::define_static_array(
            nonstatic_data_members_of(^^Input, std::meta::access_context::current()))) {
            sql_value<member_type<m>>::bind(out, in.[:m:]);
        }
    }
}
