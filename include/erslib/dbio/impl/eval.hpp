#pragma once

// Defines

#ifdef ERS_DBIO_SAFE
#define ERS_DBIO_TRY_EVAL try
#define ERS_DBIO_CATCH_EVAL_ERRORS \
    catch (const pqxx::sql_error& e) { \
        return ers::make_error("dbio: SQL error: {}", e.what()); \
    } catch (const pqxx::argument_error& e) { \
        return ers::make_error("dbio: bad argument: {}", e.what()); \
    }
#else
#define ERS_DBIO_TRY_EVAL
#define ERS_DBIO_CATCH_EVAL_ERRORS
#endif
