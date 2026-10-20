#pragma once

// std
#include <string>
#include <vector>

// ers
#include <erslib/core/fwd.hpp>
#include <erslib/core/localization/config.hpp>
#include <erslib/core/localization/fwd.hpp>
#include <erslib/core/type/optional.hpp>

// export
#include <erslib/export.hpp>


// Definition ------------------------------------------------------------------

namespace ers::impl {
    // Every locale of one first-level folder, immutable once built.
    class ERSLIB_EXPORT Folder {
        friend class LocaleTable;


    public:
        // Texts of a key, indexed by locale id.
        using translations_t = std::vector<optional<std::string>>;
        using container_type = StringMap<translations_t>;


        // Member functions

        Folder() = default;


        // Accessors

        [[nodiscard]] std::string_view operator[](std::string_view k, size_t locale_id) const;
        [[nodiscard]] optional<std::string_view> find(std::string_view k, size_t locale_id) const noexcept;

        // The target locale, then the fallback one; nullptr if neither has 'k'.
        [[nodiscard]] const std::string* lookup(std::string_view k, size_t locale_id) const noexcept;

        [[nodiscard]] const locale_config_t& config() const noexcept;
        [[nodiscard]] const container_type& strings() const noexcept;
        [[nodiscard]] size_t size() const noexcept;


    private:
        locale_config_ptr _config;
        container_type _strings_with_locale;
    };
}
