add_executable(ers_test
    "test.cpp"
)

target_link_libraries(ers_test
    PRIVATE
        erslib
        erslib_headers
        ${COMMON_LIBRARIES}
        GTest::gtest_main
)

target_include_directories(ers_test
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/../include"
)

include(GoogleTest)
gtest_discover_tests(ers_test
    WORKING_DIRECTORY
        ${PROJECT_SOURCE_DIR}
    PROPERTIES
        VS_DEBUGGER_WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
)

set_target_properties(ers_test
    PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/test"
)
