if(NOT TARGET erslib::dbio)
    message(STATUS "Skipping stateful dbio_codegen test: erslib::dbio is not available (set ERSLIB_BUILD_DBIO=ON)")
    return()
endif()

target_compile_definitions(erslib_dbio
    PUBLIC
        ERS_DBIO_GLOBAL_QUERY_STORE
        ERS_DBIO_GLOBAL_QUERY_STORE_INIT
)

add_executable(test_stateful_dbio_codegen dbio_codegen.cpp)

if(MSVC)
    target_link_options(test_stateful_dbio_codegen PRIVATE /FORCE:MULTIPLE)
endif()

target_compile_definitions(test_stateful_dbio_codegen
    PRIVATE
        DOCTEST_CONFIG_USE_STD_HEADERS
        ERS_DBIO_SAFE
        TEST_CWD="${CMAKE_CURRENT_BINARY_DIR}"
        TEST_QUERY_DIR="${CMAKE_CURRENT_BINARY_DIR}/res/query"
)

target_link_libraries(test_stateful_dbio_codegen
    PRIVATE
        erslib::dbio
        doctest::doctest
)

dbio_generate(
    TARGET        test_stateful_dbio_codegen
    IMPORT_DIR    "${CMAKE_CURRENT_SOURCE_DIR}/schema"
    HPP_DIR       "${CMAKE_CURRENT_BINARY_DIR}/generated/include"
    CPP_DIR       "${CMAKE_CURRENT_BINARY_DIR}/generated/src"
    IN_QUERY_DIR  "${CMAKE_CURRENT_SOURCE_DIR}/res/query"
    OUT_QUERY_DIR "${CMAKE_CURRENT_BINARY_DIR}/res/query"
    FILE_LIST     "${CMAKE_CURRENT_BINARY_DIR}/generated-list.txt"
)

target_include_directories(test_stateful_dbio_codegen
    PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>"
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/src>"
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/generated>"
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/generated>"
)

add_test(NAME test_stateful_dbio_codegen COMMAND test_stateful_dbio_codegen)
