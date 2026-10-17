#pragma once

// std
#include <exception>
#include <functional>
#include <string_view>
#include <type_traits>
#include <utility>

// pqxx
#include <pqxx/pqxx>

// ers
#include <erslib/core/trait/result.hpp>
#include <erslib/core/type/result.hpp>
#include <erslib/core/util/string.hpp>
#include <erslib/flux/connection/scope.hpp>
#include <erslib/flux/result/row_generator.hpp>


// Details ---------------------------------------------------------------------

namespace flux::impl {
    class TryQueryResult;
}


template<>
struct ers::impl::as_result<flux::impl::QueryResult> {
    using type = flux::impl::TryQueryResult;
};


// Implementation --------------------------------------------------------------

namespace flux::impl {
    template<std::invocable Fn>
    auto try_invoke(Fn&& fn, std::string_view name = {}) -> ers::as_result_t<std::invoke_result_t<Fn>> {
        using R = std::invoke_result_t<Fn>;

        const auto label = name.empty() ? std::string_view("database call") : name;

        try {
            if constexpr (std::is_void_v<R>) {
                std::invoke(std::forward<Fn>(fn));
                return ers::ok;
            } else
                return std::invoke(std::forward<Fn>(fn));
        } catch (const pqxx::broken_connection& e) {
            return ers::make_crit("'{}': connection lost: {}", label, ers::util::trim(e.what()));
        } catch (const pqxx::sql_error& e) {
            return ers::make_error("'{}' failed [{}]: {}", label, e.sqlstate(), ers::util::trim(e.what()));
        } catch (const std::exception& e) {
            return ers::make_error("'{}' failed: {}", label, e.what());
        }
    }


    class TryQueryResult : public ers::Result<QueryResult> {
    public:
        using ers::Result<QueryResult>::Result;


        template<typename T>
        ers::Result<T> get_as() const {
            if (!*this)
                return error();

            return try_invoke([&] { return value().template get_as<T>(); });
        }

        template<ReadableRow T>
        ers::Result<RowGenerator<T>> view_as() const {
            if (!*this)
                return error();

            return try_invoke([&] { return value().template view_as<T>(); });
        }
    };


    template<ConnectionSource Source, ConnectionHandler Fn>
    auto try_with_conn(Source& source, Fn&& fn) {
        return try_invoke([&] { return with_conn(source, std::forward<Fn>(fn)); });
    }

    template<ConnectionSource Source, TransactionCallable Fn>
    auto try_with_tx(Source& source, Fn&& fn, std::string_view name = {}) {
        return try_invoke([&] { return with_tx(source, std::forward<Fn>(fn), name); }, name);
    }

    template<TransactionCallable Fn>
    auto try_with_tx(pqxx::connection& conn, Fn&& fn, std::string_view name = {}) {
        return try_invoke([&] { return with_tx(conn, std::forward<Fn>(fn), name); }, name);
    }

    template<typename Tx = pqxx::work, ConnectionSource Source, Executable Q>
    auto try_with_query(Source& source, std::string_view name, Q query) {
        return try_invoke([&] { return with_query<Tx>(source, name, std::move(query)); }, name);
    }

    template<typename Tx = pqxx::work, ConnectionSource Source, Executable Q>
    auto try_with_query(Source& source, Q query) {
        return try_with_query<Tx>(source, std::string_view {}, std::move(query));
    }
}
