#pragma once

// std
#include <string_view>

// ers
#include <erslib/core/localization/domain_view.hpp>
#include <erslib/core/localization/fwd.hpp>
#include <erslib/core/type/maybe.hpp>

// export
#include <erslib/export.hpp>


namespace ers::impl {
    // One locale of the whole table, pinned to the index it was taken from.
    class ERSLIB_EXPORT LocaleView {
        friend class LocaleTable;


    public:
        // Accessors

        [[nodiscard]] DomainView operator[](std::string_view domain) const;
        [[nodiscard]] optional<DomainView> find(std::string_view domain) const;

        // 'common_locale_id' for LocaleTable::common().
        [[nodiscard]] size_t locale_id() const noexcept;


    private:
        locale_index_ptr _index;
        size_t _locale_id;


        LocaleView(locale_index_ptr index, size_t locale_id) noexcept;
    };
}
