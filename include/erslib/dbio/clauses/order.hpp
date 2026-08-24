#pragma once

// std
#include <string>

// ers
#include <erslib/core/type/general.hpp>
#include <erslib/dbio/impl/clause.hpp>

// export
#include <erslib/export.hpp>


// OrderClause

namespace dbio::impl {
    enum class Order : u8 { Asc, Desc };


    class ERSLIB_EXPORT OrderClause : public IClause {
    public:
        // Member functions

        OrderClause(std::string column, Order order);


        // Executors

        ers::Status render(Context& ctx) const override;


    private:
        std::string _column;
        Order _order;
    };


    namespace clauses {
        Clause order_by(std::string column, Order order = Order::Asc);
        Clause order_by_random();
    }
}


// StableRandomClause

namespace dbio::impl {
    class ERSLIB_EXPORT StableRandomClause : public IClause {
    public:
        // Member functions

        StableRandomClause(std::string key, i64 seed);


        // Executors

        ers::Status render(Context& ctx) const override;


    private:
        std::string _key;
        i64 _seed;
    };


    namespace clauses {
        Clause order_by_stable_random(std::string key, i64 seed);
    }
}
