#pragma once

// std
#include <string_view>
#include <vector>

// ers
#include <erslib/contrib/json.hpp>
#include <erslib/core/localization.hpp>
#include <erslib/core/type/result.hpp>

// export
#include <erslib/export.hpp>


// Definition ------------------------------------------------------------------

namespace utl::impl {
    inline constexpr std::string_view l10n_json_suffix = ".l10n.json";


    ers::Result<std::vector<ers::bundle_entry_t>> ERSLIB_EXPORT flatten_l10n(const Json& bundle);

    ers::bundle_loader_t ERSLIB_EXPORT l10n_json_loader();
}


// Exports ---------------------------------------------------------------------

namespace utl {
    using impl::l10n_json_suffix;
    using impl::flatten_l10n;
    using impl::l10n_json_loader;
}
