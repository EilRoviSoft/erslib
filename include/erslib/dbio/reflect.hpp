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
    using impl::RowType;
    using impl::Entity;

    using impl::all_members;
    using impl::columns;
    using impl::declaration_members;
    using impl::table_name;
    using impl::column_name;
    using impl::column_count;
    using impl::member_type;
    using impl::sql_type_name;
    using impl::is_nullable;
    using impl::is_skipped;
    using impl::has_field;
    using impl::has_column;
    using impl::primary_key;
    using impl::pk_constraint;
    using impl::is_pk_column;
    using impl::is_identity_column;
    using impl::has_identity;
    using impl::identity_column;
    using impl::declaration_is_valid;
}


// Traits

#include <erslib/dbio/reflect/traits.hpp>
#include <erslib/dbio/reflect/value.hpp>

namespace dbio::reflect {
    using impl::sql_type;
    using impl::sql_value;
    using impl::value_binder;
}


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


// Rows

#include <erslib/dbio/reflect/row.hpp>

namespace dbio::reflect {
    using impl::ColumnIndex;

    using impl::map_columns;
    using impl::read_row;
    using impl::from_row;
}
