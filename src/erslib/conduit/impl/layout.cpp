#include "erslib/conduit/impl/layout.hpp"

// std
#include <ranges>


// Accessors

conduit::impl::LayoutView conduit::impl::Layout::bindings() const noexcept {
    if (const auto* fixed = std::get_if<LayoutView>(&_repr))
        return *fixed;
    return std::get<LayoutArray>(_repr);
}

ers::optional<size_t> conduit::impl::Layout::index_of(SlotRef slot) const noexcept {
    auto view = bindings();
    auto it = std::ranges::find_if(view, [&](const SlotBinding& candidate) {
        return same_slot(candidate.slot, slot);
    });

    if (it == view.end())
        return ers::nullopt;

    return static_cast<size_t>(it - view.begin());
}

bool conduit::impl::Layout::contains(SlotRef slot) const noexcept {
    return index_of(slot).has_value();
}


// Modifiers

std::vector<conduit::impl::SlotBinding>& conduit::impl::Layout::_materialize() {
    if (auto* owned = std::get_if<LayoutArray>(&_repr))
        return *owned;
    const auto fixed = std::get<LayoutView>(_repr);
    return _repr.emplace<LayoutArray>(fixed.begin(), fixed.end());
}

ers::Status conduit::impl::Layout::_insert_at(SlotRef anchor, const SlotBinding& binding, ptrdiff_t offset) {
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

ers::Status conduit::impl::Layout::insert_after(SlotRef anchor, const SlotBinding& binding) {
    return _insert_at(anchor, binding, 1);
}

ers::Status conduit::impl::Layout::insert_before(SlotRef anchor, const SlotBinding& binding) {
    return _insert_at(anchor, binding, 0);
}


ers::Status conduit::impl::Layout::append(SlotBinding binding) {
    if (auto s = _check_insertable(binding.slot); !s)
        return s;

    _materialize().emplace_back(binding);
    return ers::ok;
}


// Checkers

ers::Status conduit::impl::Layout::_check_insertable(SlotRef slot) const {
    if (!slot)
        return ers::make_error("Null slot.");

    if (contains(slot))
        return ers::make_error("Slot '{}' is already in the layout.", slot->name);

    return ers::ok;
}
