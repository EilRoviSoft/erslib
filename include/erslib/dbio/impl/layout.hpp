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
        using LayoutArray = std::vector<SlotBinding>;

    public:
        // Member functions

        constexpr Layout(LayoutView fixed) noexcept :
            _repr(fixed) {
        }


        // Accessors

        LayoutView bindings() const noexcept;

        ers::optional<size_t> index_of(SlotRef slot) const noexcept;

        bool contains(SlotRef slot) const noexcept;


        // Modifiers

        ers::Status insert_after(SlotRef anchor, const SlotBinding& binding);
        ers::Status insert_before(SlotRef anchor, const SlotBinding& binding);

        ers::Status append(SlotBinding slot);


    private:
        std::variant<LayoutView, LayoutArray> _repr;


        LayoutArray& _materialize();

        ers::Status _insert_at(SlotRef anchor, const SlotBinding& binding, ptrdiff_t offset);
    };
}
