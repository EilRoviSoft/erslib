#pragma once

// std
#include <memory>
#include <string>

// ers
#include <erslib/core/localization/fwd.hpp>
#include <erslib/core/type/optional.hpp>

// export
#include <erslib/export.hpp>


// Definition ------------------------------------------------------------------

namespace ers::impl {
    // One locale of one folder, keeps that folder alive
    class ERSLIB_EXPORT FolderView {
    public:
        // Member functions

        FolderView(FolderPtr folder, size_t target_locale_id) noexcept;


        // Accessors

        // valid while this view lives, or while the table still holds this folder
        [[nodiscard]] std::string_view operator[](std::string_view k) const;
        [[nodiscard]] optional<std::string_view> find(std::string_view k) const noexcept;

        // outlives any reload: shares ownership of the whole folder
        [[nodiscard]] std::shared_ptr<const std::string> share(std::string_view k) const;

        [[nodiscard]] const FolderPtr& folder() const noexcept;
        [[nodiscard]] size_t locale_id() const noexcept;


    private:
        FolderPtr _folder;
        size_t _target_locale_id;
    };
}
