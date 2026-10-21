#pragma once

// std
#include <memory>
#include <span>
#include <string>
#include <vector>

// ers
#include <erslib/core/localization/fwd.hpp>
#include <erslib/core/type/maybe.hpp>

// export
#include <erslib/export.hpp>


// Definition ------------------------------------------------------------------

namespace ers::impl {
    // One locale of one domain, keeps that domain alive.
    class ERSLIB_EXPORT DomainView {
    public:
        // Member functions

        DomainView(DomainPtr domain, size_t target_locale_id) noexcept;


        // Accessors

        [[nodiscard]] std::string_view operator[](std::string_view k) const;
        [[nodiscard]] maybe<std::string_view> find(std::string_view k) const noexcept;

        // Outlives any reload: shares ownership of the whole domain.
        [[nodiscard]] std::shared_ptr<const std::string> share(std::string_view k) const;

        [[nodiscard]] std::span<const std::string_view> keys(std::string_view prefix = {}) const;
        [[nodiscard]] std::vector<std::string_view> children(std::string_view prefix = {}) const;

        [[nodiscard]] const DomainPtr& domain() const noexcept;
        [[nodiscard]] size_t locale_id() const noexcept;


    private:
        DomainPtr _domain;
        size_t _target_locale_id;
    };
}
