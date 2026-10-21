#include "erslib/flux/query/layout.hpp"

// std
#include <algorithm>
#include <ranges>


// Implementation --------------------------------------------------------------

// Accessors

flux::impl::LayoutView flux::impl::Layout::bindings() const noexcept {
    if (const auto* fixed = std::get_if<LayoutView>(&_repr))
        return *fixed;
    return std::get<LayoutArray>(_repr);
}

ers::maybe<size_t> flux::impl::Layout::index_of(SlotRef slot) const noexcept {
    auto view = bindings();
    auto it = std::ranges::find_if(view, [&](const SlotBinding& candidate) {
        return same_slot(candidate.slot, slot);
    });

    if (it == view.end())
        return ers::none;

    return static_cast<size_t>(it - view.begin());
}

bool flux::impl::Layout::contains(SlotRef slot) const noexcept {
    return index_of(slot).has_value();
}


// Modifiers

std::vector<flux::impl::SlotBinding>& flux::impl::Layout::_materialize() {
    if (auto* owned = std::get_if<LayoutArray>(&_repr))
        return *owned;
    const auto fixed = std::get<LayoutView>(_repr);
    return _repr.emplace<LayoutArray>(fixed.begin(), fixed.end());
}

ers::Status flux::impl::Layout::_insert_at(SlotRef anchor, const SlotBinding& binding, ptrdiff_t offset) {
    if (!anchor)
        return ers::make_error("Null anchor slot.");

    if (auto s = _check_insertable(binding.slot); !s)
        return s;

    const auto at = index_of(anchor);
    if (!at)
        return ers::make_error("Anchor slot '{}' is not in the layout.", anchor->name);

    auto& owned = _materialize();
    owned.emplace(owned.begin() + static_cast<ptrdiff_t>(*at) + offset, binding);

    return ers::ok;
}

ers::Status flux::impl::Layout::insert_after(SlotRef anchor, const SlotBinding& binding) {
    return _insert_at(anchor, binding, 1);
}

ers::Status flux::impl::Layout::insert_before(SlotRef anchor, const SlotBinding& binding) {
    return _insert_at(anchor, binding, 0);
}


ers::Status flux::impl::Layout::append(SlotBinding binding) {
    if (auto s = _check_insertable(binding.slot); !s)
        return s;

    _materialize().emplace_back(binding);
    return ers::ok;
}


// Validators

ers::Status flux::impl::Layout::_check_insertable(SlotRef slot) const {
    if (!slot)
        return ers::make_error("Null slot.");

    if (contains(slot))
        return ers::make_error("Slot '{}' is already in the layout.", slot->name);

    return ers::ok;
}
