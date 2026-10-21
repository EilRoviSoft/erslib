#pragma once

// std
#include <string>
#include <string_view>

// ers
#include <erslib/core/filesystem.hpp>
#include <erslib/core/type/result.hpp>
#include <erslib/flux/migration/migration.hpp>

// export
#include <erslib/export.hpp>


namespace flux::impl {
    ERSLIB_EXPORT ers::Result<migration_t> parse_migration(std::string_view filename, std::string sql);
    ERSLIB_EXPORT ers::Result<migration_set_t> load_migrations(const fs::path& dir);
}
