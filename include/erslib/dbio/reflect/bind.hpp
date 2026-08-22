#pragma once

// std
#include <meta>

// pqxx
#include <pqxx/params>

// ers
#include <erslib/dbio/reflect/schema.hpp>
#include <erslib/dbio/traits.hpp>


namespace dbio::impl::reflect {
    template<typename T>
    void bind_params(pqxx::params& out, const T& in) {
        template for (constexpr auto m : std::define_static_array(nonstatic_data_members_of(^^T, std::meta::access_context::current()))) {
            sql_value<member_type<m>>::bind(out, in.[:m:]);
        }
    }
}
