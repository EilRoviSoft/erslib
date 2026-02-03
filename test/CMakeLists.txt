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

add_custom_command(TARGET ers_test POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "$<TARGET_FILE:erslib>"
    "$<TARGET_FILE_DIR:ers_test>"
    COMMENT "Copying erslib DLL to test directory"
)

add_test(NAME ers_test COMMAND ers_test)
