add_executable(ers_test
    "case/error.cpp"
    "case/splitting.cpp"
)

target_link_libraries(ers_test
    PRIVATE
        erslib
        Catch2::Catch2WithMain
)

target_include_directories(ers_test
    PRIVATE
        "${PROJECT_SOURCE_DIR}/include"
)

set_target_properties(ers_test
    PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/test"
)

add_test(NAME ers_test COMMAND ers_test)
