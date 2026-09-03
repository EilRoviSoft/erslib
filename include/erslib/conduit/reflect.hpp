#pragma once

#ifndef ERSLIB_HAS_REFLECTION
#  error "erslib/conduit/reflect requires C++26 static reflection (configure with ERSLIB_ENABLE_REFLECTION=ON)"
#endif


// DDL

#include <erslib/conduit/reflect/ddl.hpp>

namespace conduit::ddl {
    using impl::reflect::create_table;
}


// Declaration

#include <erslib/conduit/reflect/declaration.hpp>

namespace conduit::reflect {
    using impl::reflect::Declaration;
}


// Schema

#include <erslib/conduit/reflect/schema.hpp>

namespace conduit::reflect {
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


// Types

#include <erslib/conduit/reflect/types.hpp>

namespace conduit::reflect {
    using impl::reflect::EActionOnDelete;

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

#include <erslib/conduit/reflect/row.hpp>

namespace conduit::reflect {
    using impl::reflect::ColumnIndex;

    using impl::reflect::map_columns;
    using impl::reflect::read_row;
    using impl::reflect::from_row;
}


// Bind

#include <erslib/conduit/reflect/bind.hpp>

namespace conduit::reflect {
    using impl::reflect::bind_params;
}


// Statements

#include <erslib/conduit/reflect/statement.hpp>

namespace conduit {
    using impl::QueryResult;
}

namespace conduit::reflect {
    using impl::reflect::QueryCall;
    using impl::reflect::query_call;

    using impl::reflect::QueryFn;
    using impl::reflect::query_fn;
}
