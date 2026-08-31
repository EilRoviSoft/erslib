#pragma once

// std
#include <string>

// ers
#include <erslib/dbio/impl/clause.hpp>

// export
#include <erslib/export.hpp>


namespace dbio::impl {
    class ERSLIB_EXPORT AssignClause : public IClause {
    public:
        // Member functions

        AssignClause(std::string column, binder_t binder);


        // Executors

        ers::Status render(Context& ctx) const override;


    private:
        std::string _column;
        binder_t _binder;
    };


    namespace clauses {
        template<typename T>
        Clause assign(std::string column, T&& value) {
            return make_clause<AssignClause>(std::move(column), make_binder(std::forward<T>(value)));
        }

        Clause assign_null(std::string column);

        
        template<typename T>
        Clause set(std::string column, T&& value) {
            return assign(std::move(column), std::forward<T>(value));
        }
    }
}
