# Stateful test for the dbio codegen scripts.
#
# Needs erslib::dbio (ERSLIB_BUILD_DBIO=ON, which pulls in libpqxx).
# Skipped otherwise so the rest of the test suite still configures.

if(NOT TARGET erslib::dbio)
    message(STATUS "Skipping stateful dbio_codegen test: erslib::dbio is not available (set ERSLIB_BUILD_DBIO=ON)")
    return()
endif()

set(TEST_TARGET test_stateful_dbio_codegen)

add_executable(${TEST_TARGET} dbio_codegen.cpp)

target_link_libraries(${TEST_TARGET}
    PRIVATE
        erslib::dbio
        doctest::doctest
)

if(MSVC)
    target_link_options(${TEST_TARGET} PRIVATE /FORCE:MULTIPLE)
endif()

target_compile_definitions(${TEST_TARGET}
    PRIVATE
        DOCTEST_CONFIG_USE_STD_HEADERS
        ERS_DBIO_SAFE
        TEST_CWD="${CMAKE_CURRENT_BINARY_DIR}"
)

dbio_generate(
    TARGET     ${TEST_TARGET}
    IMPORT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/schema"
    HPP_DIR    "${CMAKE_CURRENT_BINARY_DIR}/generated/include"
    CPP_DIR    "${CMAKE_CURRENT_BINARY_DIR}/generated/src"
    QUERY_DIR  "${CMAKE_CURRENT_BINARY_DIR}/query"
)

add_test(NAME ${TEST_TARGET} COMMAND ${TEST_TARGET})
