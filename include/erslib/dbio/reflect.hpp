#pragma once

#ifndef ERSLIB_HAS_REFLECTION
#  error "erslib/dbio/reflect requires C++26 static reflection (configure with ERSLIB_ENABLE_REFLECTION=ON)"
#endif


// DDL

#include <erslib/dbio/reflect/ddl.hpp>

namespace dbio::ddl {
    using impl::reflect::create_table;
}


// Declaration

#include <erslib/dbio/reflect/declaration.hpp>

namespace dbio::reflect {
    using impl::reflect::Declaration;
}


// Schema

#include <erslib/dbio/reflect/schema.hpp>

namespace dbio::reflect {
    using impl::reflect::RowType;
    using impl::reflect::Entity;
    using impl::reflect::Statement;

    using impl::reflect::all_members;
    using impl::reflect::columns;
    using impl::reflect::declaration_members;
    using impl::reflect::table_name;
    using impl::reflect::query_label;
    using impl::reflect::column_name;
    using impl::reflect::column_count;
    using impl::reflect::member_type;
    using impl::reflect::sql_type_name;
    using impl::reflect::is_nullable;
    using impl::reflect::is_skipped;
    using impl::reflect::has_field;
    using impl::reflect::has_column;
    using impl::reflect::primary_key;
    using impl::reflect::pk_constraint;
    using impl::reflect::is_pk_column;
    using impl::reflect::is_identity_column;
    using impl::reflect::has_identity;
    using impl::reflect::identity_column;
    using impl::reflect::declaration_is_valid;
}


// Traits

#include <erslib/dbio/reflect/traits.hpp>
#include <erslib/dbio/reflect/value.hpp>

namespace dbio::reflect {
    using impl::reflect::sql_type;
    using impl::reflect::sql_value;
    using impl::reflect::value_binder;
}


// Types

#include <erslib/dbio/reflect/types.hpp>

namespace dbio::reflect {
    using impl::reflect::action_on_delete;

    using impl::reflect::Table;
    using impl::reflect::Query;

    using impl::reflect::Pk;
    using impl::reflect::Unique;
    using impl::reflect::Fk;

    using impl::reflect::Default;
    using impl::reflect::Identity;
    using impl::reflect::Column;
    using impl::reflect::Skip;
}


// Rows

#include <erslib/dbio/reflect/row.hpp>

namespace dbio::reflect {
    using impl::reflect::PlainRow;
    using impl::reflect::ColumnIndex;

    using impl::reflect::map_columns;
    using impl::reflect::read_row;
    using impl::reflect::from_row;
}


// Bind

#include <erslib/dbio/reflect/bind.hpp>

namespace dbio::reflect {
    using impl::reflect::bind_params;
}


// Statements

#include <erslib/dbio/reflect/statement.hpp>

namespace dbio::reflect {
    using impl::reflect::QueryCall;
    using impl::reflect::query_call;

    using impl::reflect::QueryFn;
    using impl::reflect::query_fn;
}
