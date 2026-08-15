#pragma once

// std
#include <memory>
#include <vector>

// ers
#include <erslib/core/type/result.hpp>
#include <erslib/dbio/impl/context.hpp>
#include <erslib/dbio/impl/slot.hpp>

// export
#include <erslib/export.hpp>


namespace dbio::impl {
    using ClausePtr = std::polymorphic<class IClause>;
    using ClauseList = std::vector<ClausePtr>;

    class ERSLIB_EXPORT IClause {
    public:
        // Member functions

        explicit IClause(SlotRef slot) :
            _slot(slot) {
        }

        virtual ~IClause() = default;


        // Accessors

        SlotRef slot() const noexcept { return _slot; }


        // Executors

        virtual ers::Status render(Context& ctx) const = 0;


    private:
        SlotRef _slot;
    };


    template<typename T, typename... Args>
    ClausePtr make_clause(Args&&... args) {
        return ClausePtr(std::in_place_type<T>, std::forward<Args>(args)...);
    }
}
