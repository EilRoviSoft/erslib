#pragma once

// ers
#include <erslib/dbio/impl/clause.hpp>

// export
#include <erslib/export.hpp>


namespace dbio::impl {
    enum class Op : uint8_t {
        Eq, // ==
        Ne, // !=
        Lt, // <
        Le, // <=
        Gt, // >
        Ge, // >=
        Like,
        ILike
    };


    class ERSLIB_EXPORT ConditionClause : public IClause {
    public:
        // Member functions

        ConditionClause(std::string column, Op op, binder_t binder);


        // Executors

        [[nodiscard]]
        ers::Status render(build_context_t& ctx) const override;


        // Misc

        [[nodiscard]]
        ClausePtr clone() const override;


    private:
        std::string _column;
        Op _op;
        binder_t _binder;
    };


    namespace clauses {
        template<typename T>
        ClausePtr where(std::string column, Op op, T&& value) {
            return std::make_unique<ConditionClause>(std::move(column), op, make_binder(std::forward<T>(value)));
        }

        template<typename T>
            requires (!std::same_as<std::remove_cvref_t<T>, Op>)
        ClausePtr where(std::string column, T&& value) {
            return where(std::move(column), Op::Eq, std::forward<T>(value));
        }
    }
}
