#pragma once

// std
#include <memory>
#include <utility>

// ers
#include <erslib/core/type/result.hpp>
#include <erslib/flux/query/fwd.hpp>
#include <erslib/flux/query/slot.hpp>
#include <erslib/flux/sql/context.hpp>
#include <erslib/flux/traits.hpp>

// export
#include <erslib/export.hpp>


namespace flux::impl {
    class ERSLIB_EXPORT IClause {
    public:
        // Member functions

        explicit IClause(SlotRef slot);

        virtual ~IClause() = default;


        // Accessors

        SlotRef slot() const noexcept { return _slot; }


        // Executors

        virtual ers::Status render(Context& ctx) const = 0;


    private:
        SlotRef _slot;
    };


    template<typename T, typename... Args>
    Clause make_clause(Args&&... args) {
        return Clause(std::in_place_type<T>, std::forward<Args>(args)...);
    }
}
