#pragma once

#ifndef ERSLIB_HAS_REFLECTION
#  error "erslib/dbio/reflect requires C++26 static reflection (configure with ERSLIB_ENABLE_REFLECTION=ON)"
#endif


// DDL

#include <erslib/dbio/reflect/ddl.hpp>

namespace dbio::ddl {
    using impl::create_table;
}


// Declaration

#include <erslib/dbio/reflect/declaration.hpp>

namespace dbio::reflect {
    using impl::Declaration;
}


// Schema

#include <erslib/dbio/reflect/schema.hpp>

namespace dbio::reflect {
    using impl::Entity;

    using impl::all_members;
    using impl::columns;
    using impl::declaration_members;
    using impl::table_name;
    using impl::column_name;
    using impl::sql_type_name;
    using impl::is_nullable;
    using impl::has_field;
    using impl::has_column;
    using impl::primary_key;
    using impl::is_identity_column;
    using impl::declaration_is_valid;
}


// Traits

#include <erslib/dbio/reflect/traits.hpp>


// Types

#include <erslib/dbio/reflect/types.hpp>

namespace dbio::reflect {
    using impl::action_on_delete;

    using impl::Table;

    using impl::Pk;
    using impl::Unique;
    using impl::Fk;

    using impl::Default;
    using impl::Identity;
    using impl::Column;
    using impl::Skip;
}
