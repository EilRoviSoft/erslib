#pragma once

// std
#include <string>

// ers
#include <erslib/conduit/impl/migration.hpp>
#include <erslib/core/type/result.hpp>
#include <erslib/core/filesystem.hpp>

// export
#include <erslib/export.hpp>


namespace conduit::impl {
    ers::Result<migration_t> ERSLIB_EXPORT parse_migration(std::string_view filename, std::string sql);
    ers::Result<migration_set_t> ERSLIB_EXPORT load_migrations(const fs::path& dir);
}
