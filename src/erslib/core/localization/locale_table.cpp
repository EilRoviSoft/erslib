#include "erslib/core/localization/locale_table.hpp"

// std
#include <algorithm>
#include <format>
#include <fstream>
#include <iterator>
#include <memory>
#include <utility>

// ers
#include <erslib/core/exception.hpp>
#include <erslib/core/hashing/rapid.hpp>
#include <erslib/core/localization/config.hpp>
#include <erslib/core/localization/domain.hpp>
#include <erslib/core/localization/key.hpp>


// Details

namespace {
    struct part_t {
        size_t locale_id;
        fs::path path;
        const ers::impl::bundle_loader_t* loader;
    };

    struct scanned_file_t {
        size_t locale_id;
        fs::path path;
        std::string relative;
        std::string key;
        const ers::impl::bundle_loader_t* loader;
    };

    struct domain_scan_t {
        std::vector<scanned_file_t> files;
        ers::impl::fingerprint_t fingerprint;
    };
}


// Utility

namespace {
    bool is_hidden(const fs::path& path) {
        return path.filename().string().starts_with('.');
    }

    std::vector<std::string> list_locales(const fs::path& root, std::string_view common_dir) {
        std::vector<std::string> out;

        for (const auto& entry : fs::directory_iterator(root)) {
            auto name = entry.path().filename().string();

            if (entry.is_directory() && !is_hidden(entry.path()) && name != common_dir)
                out.emplace_back(std::move(name));
        }

        std::ranges::sort(out);
        return out;
    }

    ers::Result<std::string> read_text(const fs::path& path) {
        std::ifstream file(path, std::ios::binary);

        if (!file) {
            return ers::make_error("Cannot open '{}'.",
                path.generic_string());
        }

        return std::string(std::istreambuf_iterator(file), {});
    }


    // "colors.l10n.json" -> the loader and "colors".
    const ers::impl::bundle_loader_t* loader_of(const ers::impl::locale_table_options_t& options, std::string_view filename) {
        for (const auto& loader : options.bundles) {
            if (filename.size() > loader.suffix.size() && filename.ends_with(loader.suffix))
                return &loader;
        }

        return nullptr;
    }

    std::string_view bundle_stem(std::string_view filename, const ers::impl::bundle_loader_t& loader) {
        return filename.substr(0, filename.size() - loader.suffix.size());
    }


    // "name@ru" -> "name", "ru"; the locale may only end the last segment.
    ers::Result<std::pair<std::string_view, std::string_view>> split_locale(std::string_view key) {
        auto at = key.find('@');

        if (at == std::string_view::npos)
            return std::pair(key, std::string_view());

        auto name = key.substr(0, at);
        auto locale = key.substr(at + 1);

        if (locale.empty() || locale.contains('@') || locale.contains('.'))
            return ers::make_error("'{}' may only end with '@<locale>'.", key);

        return std::pair(name, locale);
    }

    ers::Status check_key(std::string_view key) {
        if (key.empty() || key.starts_with('.') || key.ends_with('.') || key.contains(".."))
            return ers::make_error("'{}' has an empty segment.", key);
        return ers::ok;
    }


    ers::Result<ers::StringMap<std::vector<part_t>>> list_domains(
        const ers::impl::locale_table_options_t& options, const std::vector<std::pair<size_t, fs::path>>& sources,
        const ers::impl::locale_config_t& config
    ) {
        ers::StringMap<std::vector<part_t>> out;


        for (const auto& [locale_id, dir] : sources) {
            for (const auto& entry : fs::directory_iterator(dir)) {
                if (is_hidden(entry.path()))
                    continue;


                auto filename = entry.path().filename().string();
                const auto* loader = entry.is_regular_file() ? loader_of(options, filename) : nullptr;

                if (!entry.is_directory() && !loader)
                    continue;

                auto name = loader ? std::string(bundle_stem(filename, *loader)) : filename;


                auto& parts = out[name];

                for (const auto& part : parts) {
                    if (part.locale_id == locale_id) {
                        return ers::make_error("Domain '{}' is defined twice in '{}': '{}' and '{}'.",
                            name, config.name_of(locale_id), part.path.filename().generic_string(), filename);
                    }
                }


                parts.push_back({
                    .locale_id = locale_id,
                    .path      = entry.path(),
                    .loader    = loader,
                });
            }
        }


        return out;
    }

    ers::Result<domain_scan_t> scan_domain(
        const ers::impl::locale_table_options_t& options, const fs::path& root,
        std::string_view name, const std::vector<part_t>& parts
    ) {
        domain_scan_t out;
        out.fingerprint.path = name;


        auto touch = [&](fs::file_time_type time) {
            out.fingerprint.time = std::max(out.fingerprint.time, time);
        };

        auto add = [&](const fs::directory_entry& entry, size_t locale_id, std::string key, const ers::impl::bundle_loader_t* loader) {
            out.fingerprint.size += entry.file_size();
            out.fingerprint.files++;
            touch(entry.last_write_time());

            out.files.push_back({
                .locale_id = locale_id,
                .path      = entry.path(),
                .relative  = fs::relative(entry.path(), root).generic_string(),
                .key       = std::move(key),
                .loader    = loader,
            });
        };


        for (const auto& part : parts) {
            if (part.loader) {
                add(fs::directory_entry(part.path), part.locale_id, "", part.loader);
                continue;
            }


            touch(fs::last_write_time(part.path));

            for (auto it = fs::recursive_directory_iterator(part.path); it != fs::recursive_directory_iterator(); ++it) {
                if (is_hidden(it->path())) {
                    if (it->is_directory())
                        it.disable_recursion_pending();

                    continue;
                }

                if (it->is_directory()) {
                    touch(it->last_write_time());
                    continue;
                }

                if (!it->is_regular_file())
                    continue;


                auto relative = fs::relative(it->path(), part.path);
                auto filename = relative.filename().string();

                if (const auto* loader = loader_of(options, filename)) {
                    std::string prefix;

                    for (const auto& dir : relative.parent_path())
                        prefix += dir.string() + '.';

                    prefix += bundle_stem(filename, *loader);
                    add(*it, part.locale_id, std::move(prefix), loader);
                    continue;
                }


                add(*it, part.locale_id, ERS_TRY(ers::impl::make_localization_key(relative)), nullptr);
            }
        }


        std::ranges::sort(out.files, {}, &scanned_file_t::relative);

        for (const auto& file : out.files)
            out.fingerprint.layout = ers::RapidHash<std::string_view> {}(file.relative, out.fingerprint.layout);

        return out;
    }

    ers::Status insert_bundle(
        const ers::impl::locale_config_t& config, const scanned_file_t& file, std::string_view text,
        const std::function<ers::Status(std::string, size_t, std::string)>& insert
    ) {
        auto entries = ERS_TRY(file.loader->parse(text));


        for (auto& [raw_key, value] : entries) {
            auto [name, locale] = ERS_TRY(split_locale(raw_key));


            size_t locale_id = file.locale_id;

            if (!locale.empty()) {
                if (file.locale_id != ers::impl::common_locale_id)
                    return ers::make_error("'{}': the locale is already set by the path.", raw_key);

                auto id = config.id_of(locale);

                if (!id)
                    return ers::make_error("'{}': unknown locale '{}'.", raw_key, locale);

                locale_id = *id;
            }


            auto key = file.key.empty() ? std::string(name) : std::format("{}.{}", file.key, name);
            ERS_TRY(check_key(key));

            ERS_TRY(insert(std::move(key), locale_id, std::move(value)));
        }

        return ers::ok;
    }
}


// Member functions

ers::impl::LocaleTable::LocaleTable(locale_table_options_t options) :
    _options(std::move(options)) {
}


// Loaders

auto ers::impl::LocaleTable::reload(const fs::path& root) -> Result<reload_report_t> try {
    std::scoped_lock lock(_reloading_mutex);

    if (!fs::is_directory(root)) {
        return make_error("'{}' is not a folder.",
            root.generic_string());
    }


    // step 1: locales

    auto draft = ERS_TRY(make_locale_config(list_locales(root, _options.common_dir), _options.fallback_locale)
        .context("'{}'", root));

    auto previous = _index.get();
    bool same_config = previous && previous->config->same_as(*draft);

    locale_config_ptr config = same_config ? previous->config : draft;


    // step 2: domains

    std::vector<std::pair<size_t, fs::path>> sources;

    for (size_t id = 0; id < config->locales.size(); id++)
        sources.emplace_back(id, root / config->locales[id]);

    if (!_options.common_dir.empty() && fs::is_directory(root / _options.common_dir))
        sources.emplace_back(common_locale_id, root / _options.common_dir);


    auto domains = ERS_TRY(list_domains(_options, sources, *config)
        .context("'{}'", root));


    // step 3: keeping unchanged domains, building the rest

    locale_index_t next { .config = config };
    StringMap<fingerprint_t> next_fingerprints;
    reload_report_t report;


    for (const auto& [name, parts] : domains) {
        auto scan = ERS_TRY(scan_domain(_options, root, name, parts)
            .context("Domain '{}'", root));


        auto known = _fingerprints.find(name);
        DomainPtr existing;

        if (same_config) {
            if (auto it = previous->domains.find(name); it != previous->domains.end())
                existing = it->second;
        }


        // step 3.1: nothing touched the domain

        if (existing && known != _fingerprints.end() && scan.fingerprint.same_metadata_as(known->second)) {
            next.domains.emplace(name, std::move(existing));
            next_fingerprints.emplace(name, known->second);
            report.kept++;
            continue;
        }


        // step 3.2: reading, the domain was touched but may be the same

        std::vector<std::string> texts;
        texts.reserve(scan.files.size());


        size_t checksum = 0;

        for (const auto& file : scan.files) {
            auto text = ERS_TRY(read_text(file.path));

            checksum = RapidHash<std::string_view> {}(file.relative, checksum);
            checksum = RapidHash<std::string_view> {}(text, checksum);

            texts.emplace_back(std::move(text));
        }


        scan.fingerprint.checksum = checksum;

        if (existing && known != _fingerprints.end() && scan.fingerprint.same_content_as(known->second)) {
            next.domains.emplace(name, std::move(existing));
            next_fingerprints.emplace(name, std::move(scan.fingerprint));
            report.kept++;
            continue;
        }


        // step 3.3: building

        auto domain = std::make_shared<Domain>();
        domain->_config = config;

        auto insert = [&](std::string key, size_t locale_id, std::string text) {
            return domain->_insert(std::move(key), locale_id, std::move(text));
        };


        for (size_t i = 0; i < scan.files.size(); i++) {
            const auto& file = scan.files[i];

            auto inserted = file.loader
                ? insert_bundle(*config, file, texts[i], insert)
                : insert(file.key, file.locale_id, std::move(texts[i]));

            ERS_TRY(std::move(inserted).context("'{}'", file.relative));
        }

        domain->_seal();


        // step 3.4: validating

        if (auto verdict = _validate(name, domain); !verdict) {
            // nothing to keep, or keeping isn't allowed
            if (_options.on_invalid == EInvalidDomain::FailReload || !existing) {
                return std::move(verdict).context("Domain '{}' is invalid", name).extract_error();
            }

            // The old fingerprint stays, so the next reload checks the domain again.
            next.domains.emplace(name, std::move(existing));

            if (known != _fingerprints.end())
                next_fingerprints.emplace(name, known->second);

            report.rejected.push_back({ .domain = name, .reason = std::string(verdict.error().message()) });
            continue;
        }


        next.domains.emplace(name, std::move(domain));
        next_fingerprints.emplace(name, std::move(scan.fingerprint));
        report.loaded.emplace_back(name);
    }


    if (previous) {
        for (const auto& name : previous->domains | std::views::keys) {
            if (!next.domains.contains(name))
                report.removed.emplace_back(name);
        }
    }


    // step 4: publishing

    _index.replace(std::move(next));
    _fingerprints = std::move(next_fingerprints);

    std::ranges::sort(report.loaded);
    std::ranges::sort(report.removed);
    std::ranges::sort(report.rejected, {}, &rejected_t::domain);

    return report;
} catch (const std::exception& e) {
    return make_error("Cannot reload strings from '{}': {}",
        root.generic_string(), e.what());
}


// Validators

ers::Status ers::impl::LocaleTable::add_validator(std::string_view domain, domain_validator_t validator) {
    std::scoped_lock lock(_reloading_mutex);

    auto [it, _] = _validators.try_emplace(std::string(domain));
    it->second.push_back(std::move(validator));


    auto index = _index.get();

    if (!index)
        return ok;

    auto current = index->domains.find(domain);

    if (current == index->domains.end())
        return ok;


    auto verdict = _validate(domain, current->second);

    if (!verdict) {
        if (auto known = _fingerprints.find(domain); known != _fingerprints.end())
            _fingerprints.erase(known);

        return make_error("Domain '{}' is invalid: {}",
            domain, verdict.error().message());
    }


    return ok;
}

ers::Status ers::impl::LocaleTable::_validate(std::string_view domain, const DomainPtr& built) const {
    auto it = _validators.find(domain);

    if (it == _validators.end())
        return ok;


    for (const auto& validator : it->second) {
        try {
            ERS_TRY(validator(built));
        } catch (const std::exception& e) {
            return make_error("{}", e.what());
        }
    }


    return ok;
}


// Accessors

ers::impl::LocaleView ers::impl::LocaleTable::operator[](std::string_view locale) const {
    if (auto view = find(locale))
        return std::move(*view);

    throw make_out_of_range_error("No locale '{}'", locale);
}

ers::maybe<ers::impl::LocaleView> ers::impl::LocaleTable::find(std::string_view locale) const {
    auto index = _index.get();

    if (!index)
        return none;

    if (auto id = index->config->id_of(locale))
        return LocaleView(std::move(index), *id);

    return none;
}

ers::impl::LocaleView ers::impl::LocaleTable::common() const {
    auto index = _index.get();

    if (!index)
        throw make_out_of_range_error("No strings are loaded");

    return { std::move(index), common_locale_id };
}


auto ers::impl::LocaleTable::options() const noexcept -> const locale_table_options_t& {
    return _options;
}

ers::impl::locale_config_ptr ers::impl::LocaleTable::config() const {
    if (auto index = _index.get())
        return index->config;
    return nullptr;
}

ers::impl::locale_index_ptr ers::impl::LocaleTable::index() const {
    return _index.get();
}


std::vector<std::string> ers::impl::LocaleTable::missing() const {
    std::vector<std::string> out;


    auto index = _index.get();
    if (!index)
        return out;


    for (const auto& [name, domain] : index->domains) {
        for (const auto& [key, entry] : domain->entries()) {
            if (entry.common)
                continue;

            for (size_t id = 0; id < index->config->locales.size(); id++) {
                if (id >= entry.localized.size() || !entry.localized[id]) {
                    out.emplace_back(std::format("{}: {}/{}",
                        index->config->locales[id], name, key));
                }
            }
        }
    }


    std::ranges::sort(out);
    return out;
}
