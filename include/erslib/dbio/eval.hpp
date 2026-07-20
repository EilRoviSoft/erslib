#pragma once


namespace dbio::tag {
    struct transaction_t {};
}

namespace dbio {
    constexpr tag::transaction_t transaction_tag;
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
