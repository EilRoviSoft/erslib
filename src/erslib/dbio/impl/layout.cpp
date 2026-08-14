#include "erslib/dbio/impl/layout.hpp"

// std
#include <ranges>


// Accessors

dbio::impl::SlotView dbio::impl::Layout::slots() const noexcept {
    if (const auto* fixed = std::get_if<SlotView>(&_repr))
        return *fixed;
    return std::get<std::vector<SlotRef>>(_repr);
}

ers::optional<size_t> dbio::impl::Layout::index_of(SlotRef slot) const noexcept {
    auto view = slots();
    auto it = std::ranges::find_if(view, [&](SlotRef candidate) {
        return same_slot(candidate, slot);
    });

    if (it == view.end())
        return ers::nullopt;

    return static_cast<size_t>(it - view.begin());
}

bool dbio::impl::Layout::contains(SlotRef slot) const noexcept {
    return index_of(slot).has_value();
}


// Modifiers

std::vector<dbio::impl::SlotRef>& dbio::impl::Layout::_materialize() {
    if (auto* owned = std::get_if<std::vector<SlotRef>>(&_repr))
        return *owned;
    const auto fixed = std::get<SlotView>(_repr);
    return _repr.emplace<std::vector<SlotRef>>(fixed.begin(), fixed.end());
}

ers::Status dbio::impl::Layout::_insert_at(SlotRef anchor, SlotRef slot, ptrdiff_t offset) {
    if (!slot || !anchor)
        return ers::make_error("Null slot.");

    if (contains(slot)) {
        return ers::make_error("Slot '{}' is already in the layout.",
            slot->name);
    }

    const auto at = index_of(anchor);
    if (!at) {
        return ers::make_error("Anchor slot '{}' is not in the layout.",
            anchor->name);
    }

    auto& owned = _materialize();
    owned.emplace(owned.begin() + static_cast<ptrdiff_t>(*at) + offset, slot);

    return ers::ok;
}

ers::Status dbio::impl::Layout::insert_after(SlotRef anchor, SlotRef slot) {
    return _insert_at(anchor, slot, 1);
}

ers::Status dbio::impl::Layout::insert_before(SlotRef anchor, SlotRef slot) {
    return _insert_at(anchor, slot, 0);
}


ers::Status dbio::impl::Layout::append(SlotRef slot) {
    if (!slot)
        return ers::make_error("Null slot.");

    if (contains(slot)) {
        return ers::make_error("Slot '{}' is already in the layout.",
            slot->name);
    }

    _materialize().emplace_back(slot);
    return ers::ok;
}
