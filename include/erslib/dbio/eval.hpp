#pragma once

// std
#include <string_view>
#include <utility>

// pqxx
#include <pqxx/pqxx>

// ers
#include <erslib/core/type/error.hpp>
#include <erslib/core/type/result.hpp>


namespace dbio::tag {
    struct transaction_t {};
}

namespace dbio {
    constexpr tag::transaction_t transaction_tag;
}

namespace dbio::fn {
    // Invokes a generated dbio functor against an existing transaction.
    template<typename TFunc, typename... TArgs>
    auto eval(pqxx::dbtransaction& tnx, TArgs&&... args) {
        return TFunc {}(tnx, std::forward<TArgs>(args)...);
    }


    // Invokes a generated dbio functor inside a nested savepoint, committing on
    // success and aborting on failure. The result type is whatever the functor
    // returns (ers::Result<...> / ers::Status), so its operator bool drives the
    // commit/abort decision.
    template<typename TFunc, typename... TArgs>
    auto eval_with_transaction(pqxx::dbtransaction& tnx, std::string_view label, TArgs&&... args) {
        pqxx::subtransaction subtnx(tnx, std::string(label));

        auto s = TFunc {}(subtnx, std::forward<TArgs>(args)...);

        if (s)
            subtnx.commit();
        else
            subtnx.abort();

        return s;
    }
}


// Optional exception-to-Error translation around the raw pqxx calls in generated _dbio() bodies.
// Enabled by defining ERS_DBIO_SAFE; otherwise pqxx exceptions are propagated as usual.

#ifdef ERS_DBIO_SAFE
#define ERS_DBIO_TRY_EVAL try
#define ERS_DBIO_CATCH_EVAL_ERRORS \
    catch (const pqxx::sql_error& e) { \
        return ers::make_error(ers::Severity::Error, "dbio: SQL error: {}", e.what()); \
    } catch (const pqxx::argument_error& e) { \
        return ers::make_error(ers::Severity::Error, "dbio: bad argument: {}", e.what()); \
    }
#else
#define ERS_DBIO_TRY_EVAL
#define ERS_DBIO_CATCH_EVAL_ERRORS
#endif
