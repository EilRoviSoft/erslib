#include "erslib/core/localization/domain.hpp"

// std
#include <algorithm>
#include <format>
#include <ranges>
#include <utility>

// ers
#include <erslib/core/exception.hpp>
#include <erslib/core/localization/options.hpp>


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

    // "stripped-blue.hex" + "blue" -> "blue.hex"
    // "a.stripped-blue.hex" + "blue" -> "a.blue.hex"
    // "brand" + "x" -> none
    ers::maybe<std::string> sibling_key(std::string_view key, std::string_view name) {
        const auto leaf_at = key.rfind('.');

        if (leaf_at == std::string_view::npos)
            return ers::none;

        const auto leaf = key.substr(leaf_at + 1);
        const auto parent = key.substr(0, leaf_at);

        const auto group_at = parent.rfind('.');
        const auto prefix = group_at == std::string_view::npos ? std::string_view() : parent.substr(0, group_at + 1);

        return std::format("{}{}.{}", prefix, name, leaf);
    }

    bool has_empty_segment(std::string_view key) {
        return key.empty() || key.starts_with('.') || key.ends_with('.') || key.contains("..");
    }
}


// Accessors

std::string_view ers::impl::Domain::operator[](std::string_view k, size_t locale_id) const {
    if (const auto* text = lookup(k, locale_id))
        return *text;

    throw make_out_of_range_error("No string '{}' for locale '{}'",
        k, _config->locales.at(locale_id));
}

ers::maybe<std::string_view> ers::impl::Domain::find(std::string_view k, size_t locale_id) const noexcept {
    if (const auto* text = lookup(k, locale_id))
        return std::string_view(*text);
    return none;
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

    maybe<std::string>* slot;

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
        _sorted_keys.push_back(key);

    std::ranges::sort(_sorted_keys, segment_less);
}


ers::Status ers::impl::Domain::_resolve_references(const reference_options_t& options) {
    std::vector<std::string> keys;
    keys.reserve(_entries.size());

    for (const auto& key : _entries | std::views::keys)
        keys.push_back(key);

    std::ranges::sort(keys);


    std::vector<std::string> path;

    for (const auto& key : keys) {
        const auto& entry = _entries.find(key)->second;

        if (entry.common)
        ERS_TRY(_resolve_slot(key, common_locale_id, options, path));

        for (size_t id = 0; id < entry.localized.size(); ++id) {
            if (entry.localized[id])
            ERS_TRY(_resolve_slot(key, id, options, path));
        }
    }

    return ok;
}

ers::Status ers::impl::Domain::_resolve_slot(
    std::string_view key,
    size_t locale_id,
    const reference_options_t& options,
    std::vector<std::string>& path
) {
    std::string& text = **_slot(key, locale_id);

    if (text.empty() || text.front() != options.marker)
        return ok;

    if (text.size() > 1 && text[1] == options.marker) {
        text.erase(0, 1);
        return ok;
    }


    auto label = locale_id == common_locale_id
        ? std::string(key)
        : std::format("{}@{}", key, _config->name_of(locale_id));

    if (std::ranges::contains(path, label)) {
        return make_error("Reference cycle: {} -> {}.",
            path | std::views::join_with(std::string_view(" -> ")) | std::ranges::to<std::string>(), label);
    }

    if (path.size() >= options.max_depth)
        return make_error("'{}': references are nested deeper than {}.", label, options.max_depth);


    auto name = std::string(text.substr(1));

    if (name.empty())
        return make_error("'{}': '{}' has no name after it.", label, options.marker);


    std::string target_key;

    if (name.starts_with('.')) {
        target_key = name.substr(1);
    } else if (name.contains('.')) {
        target_key = name;
    } else if (auto sibling = sibling_key(key, name)) {
        target_key = std::move(*sibling);
    } else {
        return make_error("'{}': '{}{}' is relative and the key has no group, write '{}.{}' as a full key.",
            label, options.marker, name, options.marker, name);
    }

    if (has_empty_segment(target_key))
        return make_error("'{}': '{}{}' is not a key.", label, options.marker, name);


    auto target = _entries.find(target_key);
    auto supplier = target != _entries.end()
        ? _supplier(target->second, locale_id)
        : none;

    if (!supplier) {
        return make_error("'{}': '{}{}' refers to '{}', which has no text for {}.",
            label, options.marker, name, target_key, _config->name_of(locale_id));
    }


    path.push_back(label);
    ERS_TRY(_resolve_slot(target->first, *supplier, options, path));
    path.pop_back();

    text = **_slot(target_key, *supplier);

    return ok;
}

ers::maybe<std::string>* ers::impl::Domain::_slot(std::string_view key, size_t locale_id) noexcept {
    auto it = _entries.find(key);

    if (it == _entries.end())
        return nullptr;


    auto& entry = it->second;

    if (locale_id == common_locale_id)
        return &entry.common;


    return locale_id < entry.localized.size()
        ? &entry.localized[locale_id]
        : nullptr;
}

ers::maybe<unsigned long> ers::impl::Domain::_supplier(const entry_t& entry, size_t locale_id) const noexcept {
    auto has = [&](size_t id) {
        return id < entry.localized.size() && entry.localized[id].has_value();
    };


    if (locale_id == common_locale_id)
        return entry.common ? maybe<size_t>(common_locale_id) : none;

    if (has(locale_id))
        return locale_id;

    if (entry.common)
        return common_locale_id;

    if (has(_config->fallback_locale_id))
        return _config->fallback_locale_id;


    return none;
}
