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

ers::impl::FolderView ers::impl::LocaleView::operator[](std::string_view folder) const {
    if (auto view = find(folder))
        return std::move(*view);

    throw make_out_of_range_error("No folder '{}'",
        folder);
}

ers::optional<ers::impl::FolderView> ers::impl::LocaleView::find(std::string_view folder) const {
    if (auto it = _index->folders.find(folder); it != _index->folders.end())
        return FolderView(it->second, _locale_id);
    return nullopt;
}

size_t ers::impl::LocaleView::locale_id() const noexcept {
    return _locale_id;
}
