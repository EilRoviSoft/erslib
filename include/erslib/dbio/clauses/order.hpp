#pragma once

// std
#include <cstdint>
#include <string>

// ers
#include <erslib/dbio/impl/clause.hpp>

// export
#include <erslib/export.hpp>


namespace dbio::impl {
    enum class Order : uint8_t { Asc, Desc };


    class ERSLIB_EXPORT OrderClause : public IClause {
    public:
        // Member functions

        OrderClause(std::string column, Order order);


        // Executors

        [[nodiscard]]
        ers::Status render(Context& ctx) const override;


        // Misc

        [[nodiscard]]
        ClausePtr clone() const override;


    private:
        std::string _column;
        Order _order;
    };


    namespace clauses {
        ClausePtr order_by(std::string column, Order order = Order::Asc);
        ClausePtr order_by_random();
    }
}
