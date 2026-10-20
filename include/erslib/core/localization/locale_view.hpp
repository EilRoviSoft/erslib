#pragma once

// std
#include <string_view>

// ers
#include <erslib/core/localization/folder_view.hpp>
#include <erslib/core/localization/fwd.hpp>
#include <erslib/core/type/optional.hpp>

// export
#include <erslib/export.hpp>


// Definition ------------------------------------------------------------------

namespace ers::impl {
    // One locale of the whole table, pinned to the index it was taken from.
    class ERSLIB_EXPORT LocaleView {
        friend class LocaleTable;


    public:
        // Accessors

        [[nodiscard]] FolderView operator[](std::string_view folder) const;
        [[nodiscard]] optional<FolderView> find(std::string_view folder) const;

        [[nodiscard]] size_t locale_id() const noexcept;


    private:
        locale_index_ptr _index;
        size_t _locale_id;


        LocaleView(locale_index_ptr index, size_t locale_id) noexcept;
    };
}
