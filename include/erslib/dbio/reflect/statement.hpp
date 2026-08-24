#pragma once

// std
#include <string>
#include <utility>

// pqxx
#include <pqxx/dbtransaction>
#include <pqxx/params>

// ers
#include <erslib/core/type/result.hpp>
#include <erslib/dbio/impl/eval.hpp>
#include <erslib/dbio/impl/generator.hpp>
#include <erslib/dbio/impl/query_store.hpp>
#include <erslib/dbio/reflect/bind.hpp>
#include <erslib/dbio/reflect/row.hpp>
#include <erslib/dbio/reflect/schema.hpp>


namespace dbio::impl::reflect {
    template<Statement Tag>
    class QueryCall {
    public:
        using Input = typename Declaration<Tag>::Input;
        using Output = typename Declaration<Tag>::Output;

        explicit QueryCall(Input input) :
            _input(std::move(input)) {
        }


        ers::Result<RowGenerator<Output>> exec(pqxx::dbtransaction& tx) const {
            auto r = _exec_raw(tx);
            if (!r)
                return r.error();

            return RowGenerator<Output>(std::move(*r));
        }

        ers::Result<Output> exec_one(pqxx::dbtransaction& tx) const {
            auto r = _exec_raw(tx);
            if (!r)
                return r.error();
            return read_one<Output>(*r);
        }


    private:
        Input _input;


        ers::Result<pqxx::result> _exec_raw(pqxx::dbtransaction& tx) const ERS_DBIO_TRY_EVAL {
            const auto sql = queries.get(std::string(query_label<Tag>));
            if (!sql)
                return ers::make_error("dbio: no query registered for label '{}'", query_label<Tag>);

            pqxx::params params;
            bind_params(params, _input);

            return tx.exec(*sql, params);
        } ERS_DBIO_CATCH_EVAL_ERRORS
    };


    template<Statement Tag, typename... Args>
    QueryCall<Tag> query_call(Args&&... args) {
        return QueryCall<Tag>(typename Declaration<Tag>::Input(std::forward<Args>(args)...));
    }


    template<typename Tag>
    struct QueryFn {
        template<typename... Args>
        auto operator()(Args&&... args) const {
            return query_call<Tag>(std::forward<Args>(args)...);
        }
    };

    template<typename Tag>
    inline constexpr QueryFn<Tag> query_fn {};
}
