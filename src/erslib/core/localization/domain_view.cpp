#include "erslib/core/localization/domain_view.hpp"

// std
#include <utility>

// ers
#include <erslib/core/localization/domain.hpp>


// Member functions

ers::impl::DomainView::DomainView(DomainPtr domain, size_t target_locale_id) noexcept :
    _domain(std::move(domain)),
    _target_locale_id(target_locale_id) {
}


// Accessors

std::string_view ers::impl::DomainView::operator[](std::string_view k) const {
    return (*_domain)[k, _target_locale_id];
}

ers::optional<std::string_view> ers::impl::DomainView::find(std::string_view k) const noexcept {
    return _domain->find(k, _target_locale_id);
}


std::shared_ptr<const std::string> ers::impl::DomainView::share(std::string_view k) const {
    if (const auto* text = _domain->lookup(k, _target_locale_id))
        return { _domain, text };
    return nullptr;
}


std::span<const std::string_view> ers::impl::DomainView::keys(std::string_view prefix) const {
    return _domain->keys(prefix);
}

std::vector<std::string_view> ers::impl::DomainView::children(std::string_view prefix) const {
    return _domain->children(prefix);
}


const ers::impl::DomainPtr& ers::impl::DomainView::domain() const noexcept {
    return _domain;
}

size_t ers::impl::DomainView::locale_id() const noexcept {
    return _target_locale_id;
}
