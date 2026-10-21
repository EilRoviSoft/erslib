#pragma once

// std
#include <string>
#include <vector>

// ers
#include <erslib/core/fwd.hpp>
#include <erslib/core/localization/fwd.hpp>
#include <erslib/core/type/maybe.hpp>
#include <erslib/core/type/result.hpp>

// export
#include <erslib/export.hpp>


// Definition ------------------------------------------------------------------

namespace ers::impl {
    inline constexpr size_t common_locale_id = std::numeric_limits<size_t>::max();


    struct ERSLIB_EXPORT locale_config_t {
        std::vector<std::string> locales;
        StringMap<size_t> ids;
        size_t fallback_locale_id = 0;


        [[nodiscard]] maybe<size_t> id_of(std::string_view locale) const noexcept;

        // "<common>" for 'common_locale_id'.
        [[nodiscard]] std::string_view name_of(size_t locale_id) const noexcept;


        // Same locales with same ids and the same fallback.
        [[nodiscard]] bool same_as(const locale_config_t& other) const noexcept;
    };


    // Ids follow the order of 'locales'.
    Result<locale_config_ptr> ERSLIB_EXPORT make_locale_config(std::vector<std::string> locales, std::string_view fallback_locale);
}
