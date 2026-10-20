#pragma once

// std
#include <mutex>
#include <string>
#include <vector>

// ers
#include <erslib/core/filesystem.hpp>
#include <erslib/core/fwd.hpp>
#include <erslib/core/localization/fingerprint.hpp>
#include <erslib/core/localization/fwd.hpp>
#include <erslib/core/localization/index.hpp>
#include <erslib/core/localization/locale_view.hpp>
#include <erslib/core/thread_safe/snapshot.hpp>
#include <erslib/core/type/optional.hpp>
#include <erslib/core/type/result.hpp>

// export
#include <erslib/export.hpp>


// Definition ------------------------------------------------------------------

namespace ers::impl {
    // "<root>/<locale>/<folder>/sub/file.txt" -> table["<locale>"]["<folder>"]["txt.sub.file"]
    class ERSLIB_EXPORT LocaleTable {
    public:
        struct reload_report_t {
            std::vector<std::string> loaded;
            std::vector<std::string> removed;
            size_t kept = 0;
        };


        // Member functions

        LocaleTable() = default;


        // Loaders

        // Rebuilds only folders whose files changed, or every folder if the locales did.
        Result<reload_report_t> reload(const fs::path& root, std::string_view fallback_locale);


        // Accessors

        [[nodiscard]] LocaleView operator[](std::string_view locale) const;
        [[nodiscard]] optional<LocaleView> find(std::string_view locale) const;

        [[nodiscard]] locale_config_ptr config() const;
        [[nodiscard]] locale_index_ptr index() const;

        // "<locale>: <folder>/<key>" for every text some locales have and this one lacks.
        [[nodiscard]] std::vector<std::string> missing() const;


    private:
        thread_safe::Snapshot<locale_index_t> _index;

        StringMap<fingerprint_t> _fingerprints;
        std::mutex _reloading_mutex;
    };
}
