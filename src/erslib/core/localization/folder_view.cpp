#include "erslib/core/localization/folder_view.hpp"

// std
#include <utility>

// ers
#include <erslib/core/localization/folder.hpp>


// Member functions

ers::impl::FolderView::FolderView(FolderPtr folder, size_t target_locale_id) noexcept :
    _folder(std::move(folder)),
    _target_locale_id(target_locale_id) {
}


// Accessors

std::string_view ers::impl::FolderView::operator[](std::string_view k) const {
    return (*_folder)[k, _target_locale_id];
}

ers::optional<std::string_view> ers::impl::FolderView::find(std::string_view k) const noexcept {
    return _folder->find(k, _target_locale_id);
}

std::shared_ptr<const std::string> ers::impl::FolderView::share(std::string_view k) const {
    if (const auto* text = _folder->lookup(k, _target_locale_id))
        return std::shared_ptr<const std::string>(_folder, text);
    return nullptr;
}

const ers::impl::FolderPtr& ers::impl::FolderView::folder() const noexcept {
    return _folder;
}

size_t ers::impl::FolderView::locale_id() const noexcept {
    return _target_locale_id;
}
