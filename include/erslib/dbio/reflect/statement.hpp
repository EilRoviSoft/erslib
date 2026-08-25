#pragma once

// std
#include <string>
#include <utility>

// pqxx
#include <pqxx/dbtransaction>
#include <pqxx/params>

// ers
#include <erslib/dbio/impl/generator.hpp>
#include <erslib/dbio/impl/query_builder.hpp>
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


        QueryResult exec(pqxx::dbtransaction& tx) const {
            return QueryResult(_exec_raw(tx));
        }


    private:
        Input _input;


        pqxx::result _exec_raw(pqxx::dbtransaction& tx) const {
            auto sql = queries.get(std::string(query_label<Tag>));
            if (!sql)
                throw ers::make_runtime_error("dbio: no query registered for label '{}'", query_label<Tag>);

            pqxx::params params;
            bind_params(params, _input);

            return tx.exec(*sql, params);
        }
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
