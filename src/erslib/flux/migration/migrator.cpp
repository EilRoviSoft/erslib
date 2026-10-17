#include "erslib/flux/migration/migrator.hpp"

// std
#include <algorithm>
#include <format>
#include <ranges>
#include <utility>

// ers
#include <erslib/core/util/string.hpp>
#include <erslib/flux/migration/declaration_diff.hpp>


// Implementation --------------------------------------------------------------

// Free functions

flux::impl::EMigrationEvent flux::impl::decide_outcome(EDatabaseState state, const migration_t& what, u32 baseline) {
    switch (state) {
        case EDatabaseState::Fresh:
            return what.on == EApplyOn::Existing ? EMigrationEvent::Stamped : EMigrationEvent::Applied;

        case EDatabaseState::Legacy:
            if (*what.version <= baseline)
                return EMigrationEvent::Stamped;

            [[fallthrough]];
        case EDatabaseState::Tracked:
            return what.on == EApplyOn::Fresh ? EMigrationEvent::Skipped : EMigrationEvent::Applied;
    }

    std::unreachable();
}


// Member functions

flux::impl::Migrator::Migrator(
    IMigrationLedger& ledger,
    IMigrationDatabase& database,
    const migrate_options_t& options,
    const migration_target_t& target,
    const migration_set_t& set
) :
    _ledger(ledger),
    _database(database),
    _options(options),
    _target(target),
    _set(set) {
}


// Executors

ers::Result<flux::impl::migration_report_t> flux::impl::Migrator::run() && {
    ERS_TRY(_validate_directives());


    _state = _detect();

    if (_state == EDatabaseState::Legacy) {
        if (!_options.adopt) {
            return ers::make_error(
                "Schema '{}' has tables, but flux has no records of it. "
                "Make sure the database matches the code and run once with `adopt`",
                _target.schema
            );
        }

        _add_report(EMigrationEvent::Adopted, std::string(_target.schema));
    }


    _applied = _ledger.make_applied(_target.schema);
    _recorded = _ledger.make_tables(_target.schema);


    if (_state != EDatabaseState::Fresh)
        ERS_TRY(_apply_versioned());

    ERS_TRY(_bootstrap());

    if (_state == EDatabaseState::Fresh)
        ERS_TRY(_apply_versioned());

    ERS_TRY(_reconcile_declarations());
    ERS_TRY(_apply_repeatable());


    return std::move(_report);
}


// Accessors

const flux::impl::table_declaration_t* flux::impl::Migrator::_get_declared(std::string_view table) const {
    auto it = std::ranges::find(_target.tables, table, &table_declaration_t::name);
    return it != _target.tables.end() ? &*it : nullptr;
}

u32 flux::impl::Migrator::_get_baseline() const {
    auto it = _options.baseline.find(_target.schema);
    return it != _options.baseline.end() ? it->second : 0;
}


// Generators

std::vector<std::string> flux::impl::Migrator::_make_orphans() const {
    return _recorded
        | std::views::keys
        | std::views::filter([this](std::string_view name) { return !_get_declared(name); })
        | std::ranges::to<std::vector<std::string>>();
}

std::string flux::impl::Migrator::_make_subject(const migration_t& migration) const {
    return std::format("{}/{}", _target.schema, migration.id);
}


// Validators

flux::impl::EDatabaseState flux::impl::Migrator::_detect() const {
    if (_ledger.tracks(_target.schema))
        return EDatabaseState::Tracked;

    bool has_tables = std::ranges::any_of(_target.tables, [this](const table_declaration_t& it) {
        return _database.has_table(it.name);
    });

    return has_tables ? EDatabaseState::Legacy : EDatabaseState::Fresh;
}

ers::Status flux::impl::Migrator::_validate_directives() const {
    for (const auto& migration : _set.versioned) {
        for (const auto& claim : migration.claims)
            if (!_get_declared(claim.table)) {
                return ers::make_error("'{}' claims '{}', which is not declared in code.",
                    _make_subject(migration), claim.table);
            }

        for (const auto& table : migration.drops)
            if (_get_declared(table)) {
                return ers::make_error("'{}' drops '{}', which is still declared in code.",
                    _make_subject(migration), table);
            }
    }

    return ers::ok;
}


// Modifiers

ers::Status flux::impl::Migrator::_execute(std::string_view what, std::string_view sql) try {
    _database.execute(sql);
    return ers::ok;
} catch (const std::exception& e) {
    return ers::make_error("'{}' failed: {}",
        what, ers::util::trim(e.what()));
}


ers::Status flux::impl::Migrator::_apply_versioned() {
    for (const auto& migration : _set.versioned) {
        if (auto it = _applied.find(migration.id); it != _applied.end()) {
            if (it->second != migration.checksum)
                _add_report(EMigrationEvent::Edited, _make_subject(migration));

            continue;
        }


        auto outcome = decide_outcome(_state, migration, _get_baseline());

        if (outcome == EMigrationEvent::Applied) {
            ERS_TRY(_execute(_make_subject(migration), migration.sql));

            if (_state == EDatabaseState::Tracked)
                _apply_directives(migration);
        }

        _record(migration, outcome);
    }

    return ers::ok;
}

void flux::impl::Migrator::_apply_directives(const migration_t& migration) {
    for (const auto& claim : migration.claims) {
        auto* table = _get_declared(claim.table);

        _remember(claim.table, {
            .format = declaration_format,
            .hash   = claim.hash,
            .text   = claim.hash == table->hash ? std::string(table->text) : std::string(),
        });
    }

    for (const auto& table : migration.drops)
        _forget(table);
}

ers::Status flux::impl::Migrator::_apply_repeatable() {
    for (const auto& migration : _set.repeatable) {
        auto it = _applied.find(migration.id);

        if (it != _applied.end() && it->second == migration.checksum)
            continue;

        ERS_TRY(_execute(_make_subject(migration), migration.sql));
        _record(migration, EMigrationEvent::Applied);
    }

    return ers::ok;
}


ers::Status flux::impl::Migrator::_bootstrap() {
    ERS_TRY(_execute(_target.schema, _target.create_schema));

    for (const auto& table : _target.tables)
        ERS_TRY(_execute(table.name, table.ddl));

    return ers::ok;
}

ers::Status flux::impl::Migrator::_reconcile_declarations() {
    std::vector<declaration_mismatch_t> mismatches;


    for (const auto& table : _target.tables) {
        auto recorded = _recorded.find(table.name);
        bool known = recorded != _recorded.end() && recorded->second.format == declaration_format;


        if (known && recorded->second.hash == table.hash)
            continue;

        if (known && !_try_accept(table.name)) {
            mismatches.push_back({
                .table    = std::string(table.name),
                .recorded = recorded->second.text,
                .declared = table,
            });

            continue;
        }


        _remember(table);
    }


    for (const auto& name : _make_orphans()) {
        if (_try_accept(name))
            _forget(name);
        else {
            mismatches.push_back({
                .table    = name,
                .recorded = _recorded.at(name).text,
                .declared = ers::nullopt,
            });
        }
    }


    if (!mismatches.empty())
        return ers::make_error(describe_mismatches(mismatches));

    return ers::ok;
}


void flux::impl::Migrator::_record(const migration_t& migration, EMigrationEvent outcome) {
    _ledger.record(_target.schema, migration, outcome);
    _add_report(outcome, _make_subject(migration));
}

void flux::impl::Migrator::_remember(std::string_view table, recorded_table_t what) {
    _ledger.remember(_target.schema, table, what);
    _recorded.insert_or_assign(table, std::move(what));
}

void flux::impl::Migrator::_remember(const table_declaration_t& table) {
    _remember(table.name, {
        .format = declaration_format,
        .hash   = table.hash,
        .text   = std::string(table.text),
    });
}

void flux::impl::Migrator::_forget(std::string_view table) {
    _ledger.forget(_target.schema, table);
    _recorded.erase(table);
}

bool flux::impl::Migrator::_try_accept(std::string_view table) {
    if (!_options.accept_declarations)
        return false;

    _add_report(EMigrationEvent::Accepted, std::string(table));
    return true;
}

void flux::impl::Migrator::_add_report(EMigrationEvent kind, std::string subject) {
    _report.push_back({
        .kind    = kind,
        .subject = std::move(subject)
    });
}
