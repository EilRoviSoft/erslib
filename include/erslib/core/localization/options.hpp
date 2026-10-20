#pragma once

// std
#include <functional>
#include <string>
#include <vector>

// ers
#include <erslib/core/localization/fwd.hpp>
#include <erslib/core/type/result.hpp>


namespace ers::impl {
    enum class EInvalidDomain {
        FailReload,   // The whole reload fails, nothing changes.
        KeepPrevious, // Other domains are published, the invalid one stays as it was.
    };


    struct bundle_entry_t {
        std::string key;
        std::string text;
    };

    struct bundle_loader_t {
        std::string suffix;
        std::function<Result<std::vector<bundle_entry_t>>(std::string_view text)> parse;
    };


    struct locale_table_options_t {
        std::string fallback_locale;
        std::string common_dir = "_common";
        std::vector<bundle_loader_t> bundles;
        EInvalidDomain on_invalid = EInvalidDomain::KeepPrevious;
    };


    using domain_validator_t = std::function<Status(const DomainPtr&)>;
}
