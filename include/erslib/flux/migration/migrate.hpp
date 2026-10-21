#pragma once

// std
#include <filesystem>
#include <span>
#include <string>
#include <string_view>

// pqxx
#include <pqxx/pqxx>

// ers
#include <erslib/core/filesystem.hpp>
#include <erslib/core/type/result.hpp>
#include <erslib/flux/migration/options.hpp>
#include <erslib/flux/migration/report.hpp>

// export
#include <erslib/export.hpp>


namespace flux::impl {
    ERSLIB_EXPORT std::string_view service_schema();

    ERSLIB_EXPORT ers::Result<migration_report_t> run_migrations(
        pqxx::connection& conn,
        std::span<const migration_target_t> targets,
        const std::filesystem::path& root,
        const migrate_options_t& options = {}
    );
}
