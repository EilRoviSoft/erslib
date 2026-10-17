#include "erslib/flux/migration/source.hpp"

// std
#include <algorithm>
#include <charconv>
#include <format>
#include <regex>

// ers
#include <erslib/core/convert/number.hpp>
#include <erslib/core/hashing/fnv.hpp>
#include <erslib/core/util/string.hpp>


// Details ---------------------------------------------------------------------

namespace {
    const std::regex versioned_name(R"(^(\d{4})_([a-z0-9_]+)\.(existing|fresh|always)\.sql$)");
    const std::regex repeatable_name(R"(^([a-z0-9_]+)\.repeatable\.sql$)");


    const std::regex transaction_control(
        R"(^\s*(BEGIN|COMMIT|ROLLBACK|START\s+TRANSACTION)\s*;)",
        std::regex::ECMAScript | std::regex::icase | std::regex::multiline
    );


    const std::regex directive(R"(^--\s*flux:(.*)$)",
        std::regex::ECMAScript | std::regex::multiline);

    const std::regex table_directive(R"(^\s*table\s+([a-z0-9_.]+)\s+([0-9a-f]{16})\s*$)");
    const std::regex drop_directive(R"(^\s*drop\s+([a-z0-9_.]+)\s*$)");
}


namespace {
    flux::impl::EApplyOn apply_on(std::string_view mode) {
        using enum flux::impl::EApplyOn;

        if (mode == "fresh")
            return Fresh;

        if (mode == "always")
            return Always;

        return Existing;
    }

    ers::Status read_directives(flux::impl::migration_t& out) {
        for (auto it = std::sregex_iterator(out.sql.begin(), out.sql.end(), directive);
             it != std::sregex_iterator(); ++it) {
            std::string body = (*it)[1].str();
            std::smatch parsed;

            if (std::regex_match(body, parsed, table_directive)) {
                auto hash = ers::number_from_string<u64>(parsed[2].str(), 16);
                if (!hash)
                    return hash.error();

                out.claims.push_back({
                    .table = parsed[1].str(),
                    .hash  = *hash,
                });
            } else if (std::regex_match(body, parsed, drop_directive)) {
                out.drops.emplace_back(parsed[1].str());
            } else {
                return ers::make_error("'{}': unknown directive '{}'.",
                    out.id, ers::util::trim(body));
            }
        }


        bool has_directives = !out.claims.empty() || !out.drops.empty();

        if (has_directives && out.on != flux::impl::EApplyOn::Existing) {
            return ers::make_error("'{}': table directives belong to .existing.sql migrations only.",
                out.id);
        }


        return ers::ok;
    }
}


// Implementation --------------------------------------------------------------

ers::Result<flux::impl::migration_t> flux::impl::parse_migration(std::string_view filename, std::string sql) {
    std::string name(filename);

    std::smatch match;
    migration_t out;


    std::erase(sql, '\r');


    if (std::regex_match(name, match, versioned_name)) {
        out.id = std::format("{}_{}", match[1].str(), match[2].str());


        auto version = ers::number_from_string<u32>(match[1].str());
        ERS_TRY(version);

        out.version = *version;


        out.on = apply_on(match[3].str());
    } else if (std::regex_match(name, match, repeatable_name)) {
        out.id = match[1].str();
        out.on = EApplyOn::Repeatable;
    } else {
        return ers::make_error("'{}' is not a migration name, expected 'NNNN_name.{{existing|fresh|always}}.sql' or 'name.repeatable.sql'.",
            filename);
    }


    if (std::regex_search(sql, transaction_control)) {
        return ers::make_error("'{}': '{{BEGIN|COMMIT|ROLLBACK}}' are not allowed, the whole run is one transaction.",
            out.id);
    }


    out.checksum = ers::Fnv1AHash<std::string_view> {}(sql);
    out.sql = std::move(sql);

    ERS_TRY(read_directives(out));
    return out;
}

ers::Result<flux::impl::migration_set_t> flux::impl::load_migrations(const fs::path& dir) {
    migration_set_t out;

    if (!fs::is_directory(dir))
        return out;


    for (const auto& entry : fs::directory_iterator(dir)) {
        if (!entry.is_regular_file() || entry.path().extension() != ".sql")
            continue;


        auto migration = parse_migration(
            entry.path().filename().string(),
            ers::util::read_file(entry.path())
        );

        ERS_TRY(migration);


        auto& bucket = migration->version ? out.versioned : out.repeatable;
        bucket.emplace_back(std::move(*migration));
    }


    std::ranges::sort(out.versioned, {}, &migration_t::version);
    std::ranges::sort(out.repeatable, {}, &migration_t::id);


    if (auto duplicate = std::ranges::adjacent_find(out.versioned, {}, &migration_t::version);
        duplicate != out.versioned.end()) {
        return ers::make_error("'{}': version {} is used by more than one migration.",
            dir.string(), *duplicate->version);
    }


    return out;
}
