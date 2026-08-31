#pragma once

// std
#include <filesystem>
#include <string>

// pqxx
#include <pqxx/pqxx>

// ers
#include <erslib/core/type/result.hpp>
#include <erslib/dbio/impl/connection_pool.hpp>
#include <erslib/dbio/impl/options.hpp>
#include <erslib/dbio/impl/query_builder.hpp>
#include <erslib/dbio/impl/query_store.hpp>

// export
#include <erslib/export.hpp>


namespace dbio::impl {
    class ERSLIB_EXPORT Database {
    public:
        using Connection = ConnectionPool::Connection;


        // Member functions

        explicit Database(std::string connection_string, pool_options_t pool_opts = {});


        // Accessors

        ers::Result<Connection> acquire();


        template<typename Fn>
            requires ConnectionHandler<Fn>
        auto with_conn(Fn&& fn) {
            auto conn = _pool->acquire_or_throw();
            return std::forward<Fn>(fn)(*conn);
        }

        template<typename Fn>
            requires TransactionCallable<Fn>
        auto with_tx(Fn&& fn, std::string_view name = {}) {
            using Tx = transaction_arg_t<Fn>;
            using return_type = std::invoke_result_t<Fn, Tx&>;


            auto conn = _pool->acquire_or_throw();
            Tx tx(*conn, name);

            if constexpr (std::is_void_v<return_type>) {
                std::forward<Fn>(fn)(tx);
                tx.commit();
            } else {
                auto result = std::forward<Fn>(fn)(tx);
                tx.commit();
                return result;
            }
        }


        template<typename Tx = pqxx::work, typename Q>
            requires (Executable<Q> || ExecutableWithOutput<Q>)
        auto with_query(std::string_view name, Q query) {
            return this->_with_query<Tx>(name, std::move(query));
        }

        template<typename Tx = pqxx::work, typename Q>
            requires (Executable<Q> || ExecutableWithOutput<Q>)
        auto with_query(Q query) {
            return this->_with_query<Tx>("", std::move(query));
        }


        // Modifiers

        void maintain();


    private:
        ers::shared_ptr<ConnectionPool> _pool;


        template<typename Tx = pqxx::work, Executable Q>
            requires (!ExecutableWithOutput<Q>)
        QueryResult _with_query(std::string_view name, Q query) {
            auto conn = _pool->acquire_or_throw();
            Tx tx(*conn, name);

            auto result = query.exec(tx);
            tx.commit();

            return result;
        }

        template<typename Tx = pqxx::work, ExecutableWithOutput Q>
        typename Q::Output _with_query(std::string_view name, Q query) {
            auto conn = _pool->acquire_or_throw();
            Tx tx(*conn, name);

            auto result = query.exec(tx).template get_as<typename Q::Output>();
            tx.commit();

            return result;
        }
    };
}
