#include "erslib/core/localization/config.hpp"

// std
#include <memory>
#include <utility>


// locale_config_t

ers::maybe<size_t> ers::impl::locale_config_t::id_of(std::string_view locale) const noexcept {
    if (auto it = ids.find(locale); it != ids.end())
        return it->second;
    return none;
}

std::string_view ers::impl::locale_config_t::name_of(size_t locale_id) const noexcept {
    if (locale_id < locales.size())
        return locales[locale_id];
    return "<common>";
}


bool ers::impl::locale_config_t::same_as(const locale_config_t& other) const noexcept {
    return locales == other.locales && fallback_locale_id == other.fallback_locale_id;
}


// Factories

ers::Result<ers::impl::locale_config_ptr> ers::impl::make_locale_config(std::vector<std::string> locales, std::string_view fallback_locale) {
    if (locales.empty())
        return make_error("There are no locales.");

    auto out = std::make_shared<locale_config_t>();
    out->locales = std::move(locales);

    for (size_t id = 0; id < out->locales.size(); ++id)
        if (!out->ids.try_emplace(out->locales[id], id).second) {
            return make_error("Locale '{}' is listed twice.",
                out->locales[id]);
        }


    auto fallback = out->id_of(fallback_locale);

    if (!fallback) {
        return make_error("The fallback locale '{}' is not among the locales.",
            fallback_locale);
    }

    out->fallback_locale_id = *fallback;


    return locale_config_ptr(std::move(out));
}
