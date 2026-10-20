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
#include <erslib/core/localization/folder.hpp>
#include <erslib/core/localization/key.hpp>


// Details

namespace {
    struct scanned_file_t {
        size_t locale_id;
        fs::path path;
        std::string relative;
    };

    struct folder_scan_t {
        std::vector<scanned_file_t> files;
        ers::impl::fingerprint_t fingerprint;
    };
}


// Utility

namespace {
    bool is_hidden(const fs::path& path) {
        return path.filename().string().starts_with('.');
    }

    std::vector<std::string> list_dirs(const fs::path& dir) {
        std::vector<std::string> out;

        for (const auto& entry : fs::directory_iterator(dir)) {
            if (entry.is_directory() && !is_hidden(entry.path()))
                out.emplace_back(entry.path().filename().string());
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

    // Metadata only, nothing is read; the checksum stays empty.
    folder_scan_t scan_folder(const fs::path& root, const ers::impl::locale_config_t& config, std::string_view folder) {
        folder_scan_t out;
        out.fingerprint.path = folder;


        auto touch = [&](fs::file_time_type time) {
            out.fingerprint.time = std::max(out.fingerprint.time, time);
        };


        for (size_t id = 0; id < config.locales.size(); id++) {
            const auto dir = root / config.locales[id] / folder;

            if (!fs::is_directory(dir))
                continue;

            touch(fs::last_write_time(dir));


            for (auto it = fs::recursive_directory_iterator(dir); it != fs::recursive_directory_iterator(); ++it) {
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


                out.fingerprint.size += it->file_size();
                out.fingerprint.files++;
                touch(it->last_write_time());


                out.files.push_back({
                    .locale_id = id,
                    .path      = it->path(),
                    .relative  = fs::relative(it->path(), root).generic_string(),
                });
            }
        }


        std::ranges::sort(out.files, {}, &scanned_file_t::relative);
        return out;
    }
}


// Loaders

auto ers::impl::LocaleTable::reload(const fs::path& root, std::string_view fallback_locale) -> Result<reload_report_t> try {
    std::lock_guard lock(_reloading_mutex);

    if (!fs::is_directory(root)) {
        return make_error("'{}' is not a folder.",
            root.generic_string());
    }


    // step 1: locales

    auto draft = make_locale_config(list_dirs(root), fallback_locale);

    if (!draft) {
        return make_error("'{}': {}",
            root.generic_string(), draft.error().message());
    }


    auto previous = _index.get();
    bool same_config = previous && previous->config->same_as(**draft);

    // Unchanged locales keep the old config, so kept folders stay consistent with it.
    locale_config_ptr config = same_config ? previous->config : std::move(*draft);


    // step 2: first-level folders

    StringSet names;

    for (const auto& locale : config->locales) {
        for (auto& folder : list_dirs(root / locale))
            names.emplace(std::move(folder));
    }


    // step 3: keeping unchanged folders, building the rest

    locale_index_t next { .config = config };
    StringMap<fingerprint_t> next_fingerprints;
    reload_report_t report;


    for (const auto& name : names) {
        auto scan = scan_folder(root, *config, name);

        auto known = _fingerprints.find(name);
        FolderPtr kept;

        if (same_config && known != _fingerprints.end()) {
            if (auto it = previous->folders.find(name); it != previous->folders.end())
                kept = it->second;
        }


        // step 3.1: nothing touched the folder

        if (kept && scan.fingerprint.same_metadata_as(known->second)) {
            next.folders.emplace(name, std::move(kept));
            next_fingerprints.emplace(name, known->second);
            report.kept++;
            continue;
        }


        // step 3.2: reading, the folder was touched but may be the same

        std::vector<std::string> texts;
        texts.reserve(scan.files.size());


        size_t checksum = 0;

        for (const auto& file : scan.files) {
            auto text = read_text(file.path);
            ERS_TRY(text);

            checksum = RapidHash<std::string_view> {}(file.relative, checksum);
            checksum = RapidHash<std::string_view> {}(*text, checksum);

            texts.emplace_back(std::move(*text));
        }


        scan.fingerprint.checksum = checksum;

        if (kept && scan.fingerprint.same_content_as(known->second)) {
            next.folders.emplace(name, std::move(kept));
            next_fingerprints.emplace(name, std::move(scan.fingerprint));
            report.kept++;
            continue;
        }


        // step 3.3: building

        auto folder = std::make_shared<Folder>();
        folder->_config = config;

        for (size_t i = 0; i < scan.files.size(); i++) {
            const auto& file = scan.files[i];


            auto key = make_localization_key(fs::relative(file.path, root / config->locales[file.locale_id] / name));
            ERS_TRY(key);


            auto& translations = folder->_strings_with_locale[*key];
            translations.resize(config->locales.size());

            if (translations[file.locale_id]) {
                return make_error("Two files in '{}/{}' are both named '{}'.",
                    config->locales[file.locale_id], name, *key);
            }

            translations[file.locale_id] = std::move(texts[i]);
        }

        next.folders.emplace(name, std::move(folder));
        next_fingerprints.emplace(name, std::move(scan.fingerprint));
        report.loaded.emplace_back(name);
    }


    if (previous) {
        for (const auto& name : previous->folders | std::views::keys) {
            if (!next.folders.contains(name))
                report.removed.emplace_back(name);
        }
    }


    // step 4: publishing

    _index.replace(std::move(next));
    _fingerprints = std::move(next_fingerprints);

    std::ranges::sort(report.loaded);
    std::ranges::sort(report.removed);

    return report;
} catch (const std::exception& e) {
    return make_error("Cannot reload strings from '{}': {}", root.generic_string(), e.what());
}


// Accessors

ers::impl::LocaleView ers::impl::LocaleTable::operator[](std::string_view locale) const {
    if (auto view = find(locale))
        return std::move(*view);

    throw make_out_of_range_error("No locale '{}'", locale);
}

ers::optional<ers::impl::LocaleView> ers::impl::LocaleTable::find(std::string_view locale) const {
    auto index = _index.get();

    if (!index)
        return nullopt;

    if (auto id = index->config->id_of(locale))
        return LocaleView(std::move(index), *id);

    return nullopt;
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

    for (const auto& [name, folder] : index->folders) {
        for (const auto& [key, translations] : folder->strings()) {
            for (size_t id = 0; id < translations.size(); id++) {
                if (!translations[id]) {
                    out.emplace_back(std::format("{}: {}/{}",
                        index->config->locales[id], name, key));
                }
            }
        }
    }

    std::ranges::sort(out);
    return out;
}
