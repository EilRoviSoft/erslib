add_executable(ers_test
    "case/error.cpp"
    "case/splitting.cpp"
)

target_link_libraries(ers_test
    PRIVATE
        erslib
        erslib_headers
        ${COMMON_LIBRARIES}
        Catch2::Catch2WithMain
)

target_include_directories(ers_test
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/../include"
)

set_target_properties(ers_test
    PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/test"
)
