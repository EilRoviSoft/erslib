#pragma once

// std
#include <span>
#include <string>
#include <string_view>
#include <vector>

// ers
#include <erslib/core/fwd.hpp>
#include <erslib/core/localization/config.hpp>
#include <erslib/core/localization/fwd.hpp>
#include <erslib/core/type/maybe.hpp>

// export
#include <erslib/export.hpp>


// Definition ------------------------------------------------------------------

namespace ers::impl {
    // Every locale of one domain, immutable once built.
    class ERSLIB_EXPORT Domain {
        friend class LocaleTable;


    public:
        struct entry_t {
            maybe<std::string> common;

            // Indexed by locale id, empty if only the common text exists.
            std::vector<maybe<std::string>> localized;
        };

        using container_type = StringMap<entry_t>;


        // Member functions

        Domain() = default;


        // '_sorted_keys' point into '_entries'.
        Domain(const Domain&) = delete;
        Domain& operator=(const Domain&) = delete;


        // Accessors

        std::string_view operator[](std::string_view k, size_t locale_id) const;
        maybe<std::string_view> find(std::string_view k, size_t locale_id) const noexcept;

        const std::string* lookup(std::string_view k, size_t locale_id) const noexcept;

        std::span<const std::string_view> keys(std::string_view prefix = {}) const;
        std::vector<std::string_view> children(std::string_view prefix = {}) const;


        const locale_config_t& config() const noexcept;
        const container_type& entries() const noexcept;
        size_t size() const noexcept;


    private:
        locale_config_ptr _config;
        container_type _entries;
        std::vector<std::string_view> _sorted_keys;


        Status _insert(std::string key, size_t locale_id, std::string text);


        // Called once every text is inserted, before the domain is shared.
        void _seal();


        Status _resolve_references(const reference_options_t& options);

        Status _resolve_slot(std::string_view key, size_t locale_id, const reference_options_t& options, std::vector<std::string>& path);

        maybe<std::string>* _slot(std::string_view key, size_t locale_id) noexcept;

        maybe<size_t> _supplier(const entry_t& entry, size_t locale_id) const noexcept;
    };
}
