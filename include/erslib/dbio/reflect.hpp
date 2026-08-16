#pragma once

#ifndef ERSLIB_HAS_REFLECTION
#  error "erslib/dbio/reflect requires C++26 static reflection (configure with ERSLIB_ENABLE_REFLECTION=ON)"
#endif


// Annotations

#include <erslib/dbio/reflect/annotations.hpp>

namespace dbio::reflect {
    using impl::OnDelete;

    using impl::Pk;
    using impl::Fk;

    using impl::Column;
    using impl::Name;
    using impl::Default;

    using impl::Nullable;
    using impl::Identity;
    using impl::Skip;
}


// DDL

#include <erslib/dbio/reflect/ddl.hpp>

namespace dbio::ddl {
    using impl::create_table;
}


// Definition

#include <erslib/dbio/reflect/definition.hpp>

namespace dbio::reflect {
    using impl::Table;
    using impl::Unique;
    using impl::Definition;
}


// Entity

#include <erslib/dbio/reflect/entity.hpp>

namespace dbio::reflect {
    using impl::entity_specs;
}


// SqlType

#include <erslib/dbio/reflect/sql_type.hpp>


// Traits

#include <erslib/dbio/reflect/traits.hpp>

namespace dbio::reflect {
    using impl::Entity;

    using impl::has;
    using impl::has_text;
    using impl::text_of;

    using impl::columns;
    using impl::entity_members;
    using impl::column_count;
    using impl::unique_count;
    using impl::column_names;
    using impl::column_name;
    using impl::sql_type_of;
    using impl::is_nullable_column;

    using impl::table_name;
    using impl::primary_key;
    using impl::unique_groups;
}
