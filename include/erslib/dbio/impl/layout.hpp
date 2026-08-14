#pragma once

// std
#include <span>
#include <variant>
#include <vector>

// ers
#include <erslib/core/type/optional.hpp>
#include <erslib/dbio/impl/slot.hpp>

// export
#include <erslib/export.hpp>


namespace dbio::impl {
    class ERSLIB_EXPORT Layout {
    public:
        // Member functions

        constexpr Layout(SlotView fixed) noexcept :
            _repr(fixed) {
        }


        // Accessors

        SlotView slots() const noexcept;

        ers::optional<size_t> index_of(SlotRef slot) const noexcept;

        bool contains(SlotRef slot) const noexcept;


        // Modifiers

        ers::Status insert_after(SlotRef anchor, SlotRef slot);
        ers::Status insert_before(SlotRef anchor, SlotRef slot);

        ers::Status append(SlotRef slot);


    private:
        std::variant<SlotView, std::vector<SlotRef>> _repr;


        std::vector<SlotRef>& _materialize();

        ers::Status _insert_at(SlotRef anchor, SlotRef slot, ptrdiff_t offset);
    };
}
