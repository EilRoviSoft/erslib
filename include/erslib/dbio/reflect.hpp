#pragma once

#ifndef ERSLIB_HAS_REFLECTION
#  error "erslib/dbio/reflect requires C++26 static reflection (configure with ERSLIB_ENABLE_REFLECTION=ON)"
#endif


// Annotations

#include <erslib/dbio/reflect/annotations.hpp>

namespace dbio::reflect {
    using impl::Statement;
    using impl::OnDelete;

    using impl::Table;
    using impl::PrimaryKey;
    using impl::Unique;
    using impl::LayoutTag;
    using impl::ForeignKey;

    using impl::Column;
    using impl::Name;
    using impl::Default;

    using impl::InTag;
    using impl::OutTag;
    using impl::KeyTag;

    using impl::Identity;
    using impl::NotNull;
    using impl::Skip;
}


// DDL

#include <erslib/dbio/reflect/ddl.hpp>

namespace dbio::ddl {
    using impl::create_table;
}


// Query

#include <erslib/dbio/reflect/query.hpp>

namespace dbio::reflect {
    using impl::build_sql;
    using impl::sql;

    using impl::bind_params;
    using impl::params_of;
    using impl::load_row;
}


// SqlType

#include <erslib/dbio/reflect/sql_type.hpp>


// Traits

#include <erslib/dbio/reflect/traits.hpp>

namespace dbio::reflect {
    using impl::Entity;

    using impl::all_members;
    using impl::persisted;

    using impl::has;
    using impl::has_text;
    using impl::text_of;
    using impl::list_of;
    using impl::groups_of;
    using impl::list_contains;

    using impl::table_name;
    using impl::column_name;
    using impl::sql_type_of;

    using impl::column_names;
    using impl::columns_of;
    using impl::has_layout;

    using impl::declares_layout;
    using impl::statement_of;

    using impl::primary_key;
    using impl::conflict_target;
    using impl::upsert_updates;
}
