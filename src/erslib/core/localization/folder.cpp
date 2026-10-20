#include "erslib/core/localization/folder.hpp"

// ers
#include <erslib/core/exception.hpp>


// Accessors

std::string_view ers::impl::Folder::operator[](std::string_view k, size_t locale_id) const {
    if (const auto* text = lookup(k, locale_id))
        return *text;

    throw make_out_of_range_error("No string '{}' for locale '{}'",
        k, _config->locales.at(locale_id));
}

ers::optional<std::string_view> ers::impl::Folder::find(std::string_view k, size_t locale_id) const noexcept {
    if (const auto* text = lookup(k, locale_id))
        return std::string_view(*text);

    return nullopt;
}

const std::string* ers::impl::Folder::lookup(std::string_view k, size_t locale_id) const noexcept {
    auto it = _strings_with_locale.find(k);

    if (it == _strings_with_locale.end())
        return nullptr;


    const auto& texts = it->second;

    if (locale_id < texts.size() && texts[locale_id])
        return &*texts[locale_id];

    if (const auto& fallback = texts[_config->fallback_locale_id])
        return &*fallback;


    return nullptr;
}

const ers::impl::locale_config_t& ers::impl::Folder::config() const noexcept {
    return *_config;
}

auto ers::impl::Folder::strings() const noexcept -> const container_type& {
    return _strings_with_locale;
}

size_t ers::impl::Folder::size() const noexcept {
    return _strings_with_locale.size();
}
