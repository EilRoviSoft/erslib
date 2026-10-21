#pragma once

// std
#include <concepts>
#include <mutex>
#include <string>
#include <vector>

// ers
#include <erslib/core/filesystem.hpp>
#include <erslib/core/fwd.hpp>
#include <erslib/core/localization/domain_view.hpp>
#include <erslib/core/localization/fingerprint.hpp>
#include <erslib/core/localization/fwd.hpp>
#include <erslib/core/localization/index.hpp>
#include <erslib/core/localization/locale_view.hpp>
#include <erslib/core/localization/options.hpp>
#include <erslib/core/thread_safe/snapshot.hpp>
#include <erslib/core/type/maybe.hpp>
#include <erslib/core/type/result.hpp>

// export
#include <erslib/export.hpp>


namespace ers::impl {
    template<typename T>
    concept DomainAdapter =
        std::constructible_from<T, DomainView>
        && requires(const DomainPtr& domain) {
            { T::domain } -> std::convertible_to<std::string_view>;
            { T::validate(domain) } -> std::same_as<Status>;
        };
}


namespace ers::impl {
    // A domain is everything named the same right under "<root>/<locale>/" and "<root>/<common_dir>/":
    //  "<locale>/<domain>/sub/file.txt"    -> table["<locale>"]["<domain>"]["txt.sub.file"]
    //  "<locale>/<domain>/sub/b.l10n.json" -> table["<locale>"]["<domain>"]["sub.b.<key>"]
    //  "<locale>/<domain>.l10n.json"       -> table["<locale>"]["<domain>"]["<key>"]
    // Texts under "<common_dir>" belong to no locale, bundles there may set one with "<key>@<locale>".
    class ERSLIB_EXPORT LocaleTable {
    public:
        struct rejected_t {
            std::string domain;
            std::string reason;
        };

        struct reload_report_t {
            std::vector<std::string> loaded;
            std::vector<std::string> removed;
            std::vector<rejected_t> rejected;
            size_t kept = 0;
        };


        // Member functions

        explicit LocaleTable(locale_table_options_t options);


        // Loaders

        Result<reload_report_t> reload(const fs::path& root);


        // Validators

        Status add_validator(std::string_view domain, domain_validator_t validator);

        template<DomainAdapter T>
        Status attach() {
            return add_validator(T::domain, [](const DomainPtr& domain) {
                return T::validate(domain);
            });
        }


        // Accessors

        [[nodiscard]] LocaleView operator[](std::string_view locale) const;
        [[nodiscard]] maybe<LocaleView> find(std::string_view locale) const;

        [[nodiscard]] LocaleView common() const;


        // T(table[locale][T::domain])
        template<DomainAdapter T>
        [[nodiscard]] T adapt(std::string_view locale) const {
            return T((*this)[locale][T::domain]);
        }


        [[nodiscard]] const locale_table_options_t& options() const noexcept;
        [[nodiscard]] locale_config_ptr config() const;
        [[nodiscard]] locale_index_ptr index() const;


        // "<locale>: <domain>/<key>" for every text some locales have and this one lacks, common texts aren't missing.
        [[nodiscard]] std::vector<std::string> missing() const;


    private:
        const locale_table_options_t _options;

        thread_safe::Snapshot<locale_index_t> _index;

        StringMap<fingerprint_t> _fingerprints;
        StringMap<std::vector<domain_validator_t>> _validators;
        std::mutex _reloading_mutex;


        Status _validate(std::string_view domain, const DomainPtr& built) const;
    };
}
