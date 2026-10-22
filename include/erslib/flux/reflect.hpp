#pragma once

#ifndef ERSLIB_HAS_REFLECTION
#  error "erslib/flux/reflect requires C++26 static reflection (configure with ERSLIB_ENABLE_REFLECTION=ON)"
#endif


// Exports ---------------------------------------------------------------------

// DDL

#include <erslib/flux/reflect/ddl.hpp>

namespace flux::ddl {
    using impl::reflect::create_table;
    using impl::reflect::create_schema;
    using impl::reflect::schema_ddl;
}


// Declaration

#include <erslib/flux/reflect/declaration.hpp>

namespace flux::reflect {
    using impl::reflect::Declaration;
    using impl::reflect::DeclarationMetadata;
}


// Schema

#include <erslib/flux/reflect/schema.hpp>

namespace flux::reflect {
    using impl::reflect::RowType;
    using impl::reflect::Entity;
    using impl::reflect::Statement;

    using impl::reflect::all_members;
    using impl::reflect::columns;
    using impl::reflect::declaration_members;
    using impl::reflect::table_name;
    using impl::reflect::query_label;
    using impl::reflect::schema_of;
    using impl::reflect::schema_name;
    using impl::reflect::qualified_name;
    using impl::reflect::entities_of;
    using impl::reflect::SchemaType;
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
    using impl::reflect::is_read_only_column;
    using impl::reflect::soft_delete_column;
    using impl::reflect::has_soft_delete;
    using impl::reflect::pk_members;
    using impl::reflect::insert_members;
    using impl::reflect::update_members;
    using impl::reflect::has_identity;
    using impl::reflect::identity_column;
    using impl::reflect::declaration_is_valid;
    using impl::reflect::metadata_is_valid;
}


// Types

#include <erslib/flux/reflect/types.hpp>

namespace flux::reflect {
    using impl::reflect::EActionOnDelete;

    using impl::reflect::constraint_context;
    using impl::reflect::entry_defaults;
    using impl::reflect::entry_traits;
    using impl::reflect::TableConstraint;
    using impl::reflect::ColumnModifier;

    using impl::reflect::Schema;
    using impl::reflect::NoSchema;
    using impl::reflect::Table;
    using impl::reflect::Query;

    using impl::reflect::Pk;
    using impl::reflect::Unique;
    using impl::reflect::Fk;

    using impl::reflect::Default;
    using impl::reflect::Identity;
    using impl::reflect::ReadOnly;
    using impl::reflect::SoftDelete;
    using impl::reflect::Column;
    using impl::reflect::Skip;
}


// Rows

#include <erslib/flux/reflect/row.hpp>

namespace flux::reflect {
    using impl::reflect::ColumnIndex;

    using impl::reflect::map_columns;
    using impl::reflect::read_row;
    using impl::reflect::from_row;
}


// Bind

#include <erslib/flux/reflect/bind.hpp>

namespace flux::reflect {
    using impl::reflect::bind_params;
}


// Statements

#include <erslib/flux/reflect/statement.hpp>

namespace flux {
    using impl::QueryResult;
}

namespace flux::reflect {
    using impl::reflect::QueryCall;

    using impl::reflect::QueryFn;
    using impl::reflect::query_fn;
}


// CRUD

#include <erslib/flux/reflect/crud/exec.hpp>

namespace flux {
    using impl::crud::TypedColumn;
    using impl::crud::columns_of;
    using impl::crud::columns_as;
}

namespace flux::crud {
    using impl::crud::select;
    using impl::crud::select_all;
    using impl::crud::insert;
    using impl::crud::update;
    using impl::crud::upsert;
    using impl::crud::erase;
    using impl::crud::erase_forever;
    using impl::crud::restore;
    using impl::crud::find;
    using impl::crud::find_all;
    using impl::crud::where_pk;
    using impl::crud::only;
}


// Schema creation

#include <erslib/flux/reflect/create_tables.hpp>

namespace flux {
    using impl::reflect::create_tables_by_schema;
}


// Migrations

#include <erslib/flux/reflect/migrate.hpp>

namespace flux::ddl {
    using impl::reflect::canonical_declaration;
}

namespace flux::reflect {
    using impl::reflect::table_declarations;
}

namespace flux {
    using impl::reflect::migrate;
}
