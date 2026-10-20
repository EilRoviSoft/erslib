#pragma once

// std
#include <string>
#include <vector>

// ers
#include <erslib/core/fwd.hpp>
#include <erslib/core/localization/fwd.hpp>
#include <erslib/core/type/optional.hpp>
#include <erslib/core/type/result.hpp>

// export
#include <erslib/export.hpp>


// Definition ------------------------------------------------------------------

namespace ers::impl {
    struct ERSLIB_EXPORT locale_config_t {
        std::vector<std::string> locales;
        StringMap<size_t> ids;
        size_t fallback_locale_id = 0;


        [[nodiscard]] optional<size_t> id_of(std::string_view locale) const noexcept;

        // same locales with same ids and the same fallback
        [[nodiscard]] bool same_as(const locale_config_t& other) const noexcept;
    };


    // ids follow the order of 'locales'
    ERSLIB_EXPORT Result<locale_config_ptr> make_locale_config(std::vector<std::string> locales, std::string_view fallback_locale);
}


// Exports ---------------------------------------------------------------------

namespace ers {
    using impl::make_locale_config;
}
