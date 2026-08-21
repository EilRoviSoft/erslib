if(NOT TARGET erslib::dbio)
    message(STATUS "Skipping stateful dbio_reflect test: erslib::dbio is not available (set ERSLIB_BUILD_DBIO=ON)")
    return()
endif()

add_stateful_test()

target_compile_definitions(test_stateful_dbio_reflect
    PRIVATE
        ERS_DBIO_SAFE
        TEST_QUERY_DIR="${CMAKE_CURRENT_BINARY_DIR}/res/query"
)

set(ERSLIB_TEST_PG_DSN "" CACHE STRING "libpq connection string for the stateful dbio tests")

if(ERSLIB_TEST_PG_DSN)
    set_tests_properties(test_stateful_dbio_reflect
        PROPERTIES ENVIRONMENT "ERSLIB_TEST_PG_DSN=${ERSLIB_TEST_PG_DSN}"
    )
endif()
