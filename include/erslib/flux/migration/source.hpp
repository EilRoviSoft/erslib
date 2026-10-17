#pragma once

// std
#include <string>

// ers
#include <erslib/core/filesystem.hpp>
#include <erslib/core/type/result.hpp>
#include <erslib/flux/migration/migration.hpp>

// export
#include <erslib/export.hpp>


namespace flux::impl {
    ers::Result<migration_t> ERSLIB_EXPORT parse_migration(std::string_view filename, std::string sql);
    ers::Result<migration_set_t> ERSLIB_EXPORT load_migrations(const fs::path& dir);
}
