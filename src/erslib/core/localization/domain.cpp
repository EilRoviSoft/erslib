#include "erslib/core/localization/domain.hpp"

// ers
#include <erslib/core/exception.hpp>


// Utility

namespace {
    // '.' goes before any other char, so a key's subtree always follows it:
    // "black" < "black.hex" < "black.name" < "black-sequins.hex".
    constexpr bool segment_less(std::string_view lhs, std::string_view rhs) noexcept {
        auto rank = [](char c) {
            return c == '.' ? 0 : static_cast<unsigned char>(c) + 1;
        };

        return std::ranges::lexicographical_compare(lhs, rhs, {}, rank, rank);
    }
}


// Accessors

std::string_view ers::impl::Domain::operator[](std::string_view k, size_t locale_id) const {
    if (const auto* text = lookup(k, locale_id))
        return *text;

    throw make_out_of_range_error("No string '{}' for locale '{}'",
        k, _config->locales.at(locale_id));
}

ers::optional<std::string_view> ers::impl::Domain::find(std::string_view k, size_t locale_id) const noexcept {
    if (const auto* text = lookup(k, locale_id))
        return std::string_view(*text);

    return nullopt;
}


const std::string* ers::impl::Domain::lookup(std::string_view k, size_t locale_id) const noexcept {
    auto it = _entries.find(k);

    if (it == _entries.end())
        return nullptr;


    const auto& entry = it->second;

    auto localized = [&](size_t id) -> const std::string* {
        if (id < entry.localized.size() && entry.localized[id])
            return &*entry.localized[id];
        return nullptr;
    };


    if (locale_id == common_locale_id)
        return entry.common ? &*entry.common : nullptr;

    if (const auto* text = localized(locale_id))
        return text;

    if (entry.common)
        return &*entry.common;


    return localized(_config->fallback_locale_id);
}

std::span<const std::string_view> ers::impl::Domain::keys(std::string_view prefix) const {
    if (prefix.empty())
        return _sorted_keys;


    const auto group = std::string(prefix) + '.';

    auto first = std::ranges::lower_bound(_sorted_keys, std::string_view(group), segment_less);
    auto last = std::find_if_not(first, _sorted_keys.end(), [&](std::string_view key) {
        return key.starts_with(group);
    });

    return { first, last };
}

std::vector<std::string_view> ers::impl::Domain::children(std::string_view prefix) const {
    std::vector<std::string_view> out;

    const size_t skip = prefix.empty() ? 0 : prefix.size() + 1;

    for (auto key : keys(prefix)) {
        auto rest = key.substr(skip);
        auto child = rest.substr(0, rest.find('.'));

        // The same child's keys are next to each other thanks to the segment order.
        if (out.empty() || out.back() != child)
            out.push_back(child);
    }

    return out;
}


const ers::impl::locale_config_t& ers::impl::Domain::config() const noexcept {
    return *_config;
}

auto ers::impl::Domain::entries() const noexcept -> const container_type& {
    return _entries;
}

size_t ers::impl::Domain::size() const noexcept {
    return _entries.size();
}


// Building

ers::Status ers::impl::Domain::_insert(std::string key, size_t locale_id, std::string text) {
    auto [it, _] = _entries.try_emplace(std::move(key));
    auto& entry = it->second;

    optional<std::string>* slot;

    if (locale_id == common_locale_id)
        slot = &entry.common;
    else {
        entry.localized.resize(_config->locales.size());
        slot = &entry.localized[locale_id];
    }


    if (*slot) {
        return make_error("'{}' is defined twice for {}.",
            it->first, _config->name_of(locale_id));
    }

    *slot = std::move(text);
    return ok;
}

void ers::impl::Domain::_seal() {
    _sorted_keys.clear();
    _sorted_keys.reserve(_entries.size());

    for (const auto& key : _entries | std::views::keys)
        _sorted_keys.emplace_back(key);

    std::ranges::sort(_sorted_keys, segment_less);
}
