#include "erslib/core/localization/locale_view.hpp"

// std
#include <utility>

// ers
#include <erslib/core/exception.hpp>
#include <erslib/core/localization/index.hpp>


// Member functions

ers::impl::LocaleView::LocaleView(locale_index_ptr index, size_t locale_id) noexcept :
    _index(std::move(index)),
    _locale_id(locale_id) {
}


// Accessors

ers::impl::DomainView ers::impl::LocaleView::operator[](std::string_view domain) const {
    if (auto view = find(domain))
        return std::move(*view);

    throw make_out_of_range_error("No domain '{}'",
        domain);
}

ers::maybe<ers::impl::DomainView> ers::impl::LocaleView::find(std::string_view domain) const {
    if (auto it = _index->domains.find(domain); it != _index->domains.end())
        return DomainView(it->second, _locale_id);
    return none;
}

size_t ers::impl::LocaleView::locale_id() const noexcept {
    return _locale_id;
}
